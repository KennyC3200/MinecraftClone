#include "ui/Hud.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <cmath>
#include <stdexcept>

namespace mcc {

namespace {

// Matches the 4.1 core context that Window creates
constexpr const char* GLSL_VERSION = "#version 410";

// Screen pixels per crosshair texel. Keep it a whole number so the pixel art stays sharp
constexpr float CROSSHAIR_SCALE = 1.0f;

void DrawCrosshair(const Texture& crosshair) {
	const ImGuiIO& io = ImGui::GetIO();
	ImVec2 size(
		static_cast<float>(crosshair.Size().x) * CROSSHAIR_SCALE,
		static_cast<float>(crosshair.Size().y) * CROSSHAIR_SCALE);

	// Rounded down to a whole pixel, so the texels line up with screen pixels instead of blurring
	// across two of them
	ImVec2 min(
		std::floor((io.DisplaySize.x - size.x) * 0.5f),
		std::floor((io.DisplaySize.y - size.y) * 0.5f));
	ImVec2 max(min.x + size.x, min.y + size.y);

	// Texture loads images flipped vertically for OpenGL, so the UVs are flipped back here.
	// The background draw list goes under any ImGui windows, so it never covers the debug stats
	ImGui::GetBackgroundDrawList()->AddImage(
		static_cast<ImTextureID>(crosshair.Handle()), min, max, ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
}

// Screen pixels per hotbar texel. Keep it a whole number so the pixel art stays sharp
constexpr float HOTBAR_SCALE = 2.0f;

// Gap between the hotbar and the bottom of the screen, in hotbar texels
constexpr float HOTBAR_MARGIN = 2.0f;

// hotbar.png is two 22x22 slot frames side by side: unselected on the left, selected on the right.
// Both have an 18x18 see-through middle, and a 16 texel tall icon leaves a 1 texel gap inside it
constexpr float SLOT_TEXELS = 22.0f;
constexpr float ICON_TEXELS = 16.0f;

// blocks.png is a 16x16 grid of tiles
constexpr int ATLAS_TILES = 16;

struct TileUVs {
	ImVec2 m_top_left;
	ImVec2 m_top_right;
	ImVec2 m_bottom_right;
	ImVec2 m_bottom_left;
};

// UVs of one tile of blocks.png, worked out the same way as in chunk.vert. Texture flips images
// for OpenGL, so v = 1 is the top of the image
TileUVs AtlasTileUVs(int tile) {
	constexpr float N = static_cast<float>(ATLAS_TILES);
	float col = static_cast<float>(tile % ATLAS_TILES);
	float row = static_cast<float>(tile / ATLAS_TILES); // Integer division on purpose: whole rows

	float u_left = col / N;
	float u_right = (col + 1.0f) / N;
	float v_top = (N - row) / N;
	float v_bottom = (N - 1.0f - row) / N;

	return {
		ImVec2(u_left, v_top), ImVec2(u_right, v_top),
		ImVec2(u_right, v_bottom), ImVec2(u_left, v_bottom)
	};
}

// Draws a block as a small isometric cube, like Minecraft's item icons. The cube is height pixels
// tall and centred on center. The side faces are darkened, as if lit from above, so the cube
// reads as 3D
void DrawBlockIcon(
	ImDrawList* draw_list,
	const Texture& block_atlas,
	BlockId block,
	const ImVec2& center,
	float height)
{
	// In isometric view a cube edge of length s drops s / 2 and runs s * cos(30°) sideways
	float s = height * 0.5f;
	float w = s * 0.8660254f;

	// The 7 visible corners. center is the front corner where the 3 visible faces meet
	ImVec2 top(center.x, center.y - s);
	ImVec2 left(center.x - w, center.y - s * 0.5f);
	ImVec2 right(center.x + w, center.y - s * 0.5f);
	ImVec2 left_bottom(center.x - w, center.y + s * 0.5f);
	ImVec2 right_bottom(center.x + w, center.y + s * 0.5f);
	ImVec2 bottom(center.x, center.y + s);

	BlockInfo info = GetBlockInfo(block);
	TileUVs top_uvs = AtlasTileUVs(info.TileFor(Face::Up));
	TileUVs side_uvs = AtlasTileUVs(info.TileFor(Face::South));

	ImTextureID atlas = static_cast<ImTextureID>(block_atlas.Handle());

	// Each quad's corners are listed clockwise from the one that gets the tile's top left
	draw_list->AddImageQuad(atlas, top, right, center, left,
		top_uvs.m_top_left, top_uvs.m_top_right, top_uvs.m_bottom_right, top_uvs.m_bottom_left,
		IM_COL32_WHITE);
	draw_list->AddImageQuad(atlas, left, center, bottom, left_bottom,
		side_uvs.m_top_left, side_uvs.m_top_right, side_uvs.m_bottom_right, side_uvs.m_bottom_left,
		IM_COL32(204, 204, 204, 255));
	draw_list->AddImageQuad(atlas, center, right, right_bottom, bottom,
		side_uvs.m_top_left, side_uvs.m_top_right, side_uvs.m_bottom_right, side_uvs.m_bottom_left,
		IM_COL32(153, 153, 153, 255));
}

void DrawHotbar(const Hotbar& hotbar, const Texture& frames, const Texture& block_atlas) {
	const ImGuiIO& io = ImGui::GetIO();
	ImDrawList* draw_list = ImGui::GetBackgroundDrawList();

	float slot_size = SLOT_TEXELS * HOTBAR_SCALE;
	float width = slot_size * static_cast<float>(Hotbar::SIZE);

	// Bottom centre, rounded down to whole pixels so the texels stay sharp
	ImVec2 origin(
		std::floor((io.DisplaySize.x - width) * 0.5f),
		std::floor(io.DisplaySize.y - slot_size - HOTBAR_MARGIN * HOTBAR_SCALE));

	ImTextureID frames_id = static_cast<ImTextureID>(frames.Handle());

	for (std::size_t i = 0; i < Hotbar::SIZE; i++) {
		ImVec2 min(origin.x + slot_size * static_cast<float>(i), origin.y);
		ImVec2 max(min.x + slot_size, min.y + slot_size);

		// Pick the left or right half of hotbar.png, with v flipped as for the crosshair
		float u_left = i == hotbar.Selected() ? 0.5f : 0.0f;
		draw_list->AddImage(frames_id, min, max, ImVec2(u_left, 1.0f), ImVec2(u_left + 0.5f, 0.0f));

		BlockId block = hotbar.Slot(i);
		if (block == BlockId::Air) continue;

		ImVec2 center(min.x + slot_size * 0.5f, min.y + slot_size * 0.5f);
		DrawBlockIcon(draw_list, block_atlas, block, center, ICON_TEXELS * HOTBAR_SCALE);
	}
}

void DrawPhaseRow(const WorldRendererStats::Phase& phase) {
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::TextUnformatted(phase.m_name);
	ImGui::TableNextColumn();
	ImGui::Text("%.3f", phase.m_avg_ms);
	ImGui::TableNextColumn();
	ImGui::Text("%.3f", phase.m_max_ms);
}

void DrawDebugStats(const WorldRendererStats& stats) {
	// A fixed, see-through panel in the top left that never takes input, so it can't steal the
	// mouse or keyboard from the game
	constexpr ImGuiWindowFlags FLAGS =
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize |
		ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
		ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoInputs;

	ImGui::SetNextWindowPos(ImVec2(10.0f, 10.0f));
	ImGui::SetNextWindowBgAlpha(0.6f);
	if (!ImGui::Begin("Debug", nullptr, FLAGS)) {
		ImGui::End();
		return;
	}

	// Framerate is ImGui's own rolling average over the last 60 frames
	const ImGuiIO& io = ImGui::GetIO();
	ImGui::Text("%.0f FPS (%.2f ms)", io.Framerate, 1000.0f / io.Framerate);
	ImGui::Separator();

	ImGui::TextUnformatted("WorldRenderer::Update (last second)");
	if (stats.m_phases.empty()) {
		ImGui::TextDisabled("waiting for the first second...");
		ImGui::End();
		return;
	}

	// Counts are cast to int because MinGW's printf doesn't reliably support %zu
	ImGui::Text("%d frames | dirty %d | ready max %d | in flight %d",
		static_cast<int>(stats.m_frames), static_cast<int>(stats.m_dirty_cnt),
		static_cast<int>(stats.m_ready_max), static_cast<int>(stats.m_in_flight_cnt));

	constexpr ImGuiTableFlags TABLE_FLAGS = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg;
	if (ImGui::BeginTable("phases", 3, TABLE_FLAGS)) {
		ImGui::TableSetupColumn("phase");
		ImGui::TableSetupColumn("avg ms");
		ImGui::TableSetupColumn("max ms");
		ImGui::TableHeadersRow();

		for (const WorldRendererStats::Phase& phase : stats.m_phases) {
			DrawPhaseRow(phase);
		}
		DrawPhaseRow(stats.m_total);

		ImGui::EndTable();
	}

	ImGui::End();
}

}

Hud::Hud(GLFWwindow* window)
	: m_window(window)
	, m_crosshair("assets/textures/crosshair.png", GL_RGBA, GL_RGBA)
	, m_hotbar_frames("assets/textures/hotbar.png", GL_RGBA, GL_RGBA)
	, m_block_atlas("assets/textures/blocks.png", GL_RGBA, GL_RGBA)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr; // Don't write an imgui.ini next to the executable

	// The game hides and captures the cursor, so ImGui must not change its shape
	io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

	// Callbacks are installed later by InstallCallbacks(), once the game's own are set
	if (!ImGui_ImplGlfw_InitForOpenGL(m_window, false)) {
		ImGui::DestroyContext();
		throw std::runtime_error("Failed to init the ImGui GLFW backend");
	}

	if (!ImGui_ImplOpenGL3_Init(GLSL_VERSION)) {
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
		throw std::runtime_error("Failed to init the ImGui OpenGL backend");
	}
}

Hud::~Hud() {
	// The GLFW backend also restores the game's callbacks it had chained
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Hud::InstallCallbacks() {
	ImGui_ImplGlfw_InstallCallbacks(m_window);
}

void Hud::Render(const WorldRendererStats& stats, const Hotbar& hotbar) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	DrawCrosshair(m_crosshair);
	DrawHotbar(hotbar, m_hotbar_frames, m_block_atlas);

	if (m_show_debug_stats) {
		DrawDebugStats(stats);
	}

	ImGui::Render();

	// The backend saves and restores the GL state it touches (and draws filled even in wireframe)
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

}
