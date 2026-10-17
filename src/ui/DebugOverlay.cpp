#include "ui/DebugOverlay.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <stdexcept>

namespace mcc {

namespace {

// Matches the 4.1 core context that Window creates
constexpr const char* GLSL_VERSION = "#version 410";

void DrawPhaseRow(const WorldRendererStats::Phase& phase) {
	ImGui::TableNextRow();
	ImGui::TableNextColumn();
	ImGui::TextUnformatted(phase.m_name);
	ImGui::TableNextColumn();
	ImGui::Text("%.3f", phase.m_avg_ms);
	ImGui::TableNextColumn();
	ImGui::Text("%.3f", phase.m_max_ms);
}

void DrawStats(const WorldRendererStats& stats) {
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

DebugOverlay::DebugOverlay(GLFWwindow* window)
	: m_window(window)
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

DebugOverlay::~DebugOverlay() {
	// The GLFW backend also restores the game's callbacks it had chained
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void DebugOverlay::InstallCallbacks() {
	ImGui_ImplGlfw_InstallCallbacks(m_window);
}

void DebugOverlay::Render(const WorldRendererStats& stats) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	if (m_visible) {
		DrawStats(stats);
	}

	ImGui::Render();

	// The backend saves and restores the GL state it touches (and draws filled even in wireframe).
	// That backup reads GL state with glGet*, which makes the driver catch up, so skip it when hidden
	if (m_visible) {
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}
}

}
