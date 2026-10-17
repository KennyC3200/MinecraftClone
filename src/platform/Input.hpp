#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <array>

namespace mcc {

class Input final {
public:
	void BeginFrame();

	// Keyboard
	void OnKey(int key, int action);
	bool IsKeyDown(int key) const;
	bool IsKeyPressed(int key) const;

	// Mouse buttons (GLFW_MOUSE_BUTTON_*)
	void OnMouseButton(int button, int action);
	bool IsMouseDown(int button) const;
	bool IsMousePressed(int button) const;

	// Cursor
	void OnCursorMove(double x, double y);
	glm::vec2 MouseDelta() const { return m_mouse_delta; }

	void SetFirstMouse(bool val) { m_first_mouse = val; }

private:
	std::array<bool, GLFW_KEY_LAST + 1> m_down{};
	std::array<bool, GLFW_KEY_LAST + 1> m_pressed{};

	std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_mouse_down{};
	std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> m_mouse_pressed{};

	glm::vec2 m_mouse_pos{};
	glm::vec2 m_mouse_delta{};
	bool m_first_mouse = true;
};

}