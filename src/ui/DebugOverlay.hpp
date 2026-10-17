#pragma once

#include "world/WorldRenderer.hpp"

#include <GLFW/glfw3.h>

namespace mcc {

// Owns Dear ImGui (its context and the GLFW + OpenGL3 backends) and draws the on-screen debug
// stats. ImGui has one global context, so like GLFWContext this is a non-copyable, non-movable
// guard. It needs the GL context, so it must be declared after the Window and destroyed before it
class DebugOverlay final {
public:
	explicit DebugOverlay(GLFWwindow* window);
	~DebugOverlay();

	DebugOverlay(const DebugOverlay&) = delete;
	DebugOverlay& operator=(const DebugOverlay&) = delete;

	// Hooks ImGui into the window's input callbacks. Call it after the game has set its own GLFW
	// callbacks: ImGui keeps and chains the ones already installed, but a later glfwSet*Callback
	// would replace ImGui's
	void InstallCallbacks();

	void ToggleVisible() { m_visible = !m_visible; }

	// Call once per frame, after the world is drawn and before swapping buffers. Runs the ImGui
	// frame even while hidden, so its input queue keeps being drained, but only draws when visible
	void Render(const WorldRendererStats& stats);

private:
	GLFWwindow* m_window = nullptr;
	bool m_visible = true;
};

}
