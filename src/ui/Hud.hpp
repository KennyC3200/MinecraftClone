#pragma once

#include "game/Hotbar.hpp"
#include "graphics/Texture.hpp"
#include "world/WorldRenderer.hpp"

#include <GLFW/glfw3.h>

namespace mcc {

// The 2D layer drawn over the world: the crosshair, the hotbar, and the debug stats (toggled with F3).
// Owns Dear ImGui (its context and the GLFW + OpenGL3 backends), which draws all of it. ImGui has
// one global context, so like GLFWContext this is a non-copyable, non-movable guard. It needs the
// GL context, so it must be declared after the Window and destroyed before it
class Hud final {
public:
	explicit Hud(GLFWwindow* window);
	~Hud();

	Hud(const Hud&) = delete;
	Hud& operator=(const Hud&) = delete;

	// Hooks ImGui into the window's input callbacks. Call it after the game has set its own GLFW
	// callbacks: ImGui keeps and chains the ones already installed, but a later glfwSet*Callback
	// would replace ImGui's
	void InstallCallbacks();

	void ToggleDebugStats() { m_show_debug_stats = !m_show_debug_stats; }

	// Call once per frame, after the world is drawn and before swapping buffers
	void Render(const WorldRendererStats& stats, const Hotbar& hotbar);

private:
	GLFWwindow* m_window = nullptr;
	Texture m_crosshair;
	Texture m_hotbar_frames;
	Texture m_block_atlas; // For the block icons in the hotbar
	bool m_show_debug_stats = true;
};

}
