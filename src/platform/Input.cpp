#include "Input.hpp"

namespace mcc {

namespace {

bool ValidKeyRange(int key) {
	return key >= 0 && key <= GLFW_KEY_LAST;
}

}

void Input::BeginFrame() {
	m_pressed.fill(false);
	m_mouse_delta = {};
}

void Input::OnKey(int key, int action) {
	if (!ValidKeyRange(key)) return;
	if (action == GLFW_PRESS) {
		m_down[key] = true;
		m_pressed[key] = true;
	}
	if (action == GLFW_RELEASE) {
		m_down[key] = false;
	}
}

bool Input::IsKeyDown(int key) const {
	return ValidKeyRange(key) && m_down[key];
}

bool Input::IsKeyPressed(int key) const {
	return ValidKeyRange(key) && m_pressed[key];
}

void Input::OnCursorMove(double x, double y) {
	// Fixes snapping camera on focus
	if (m_first_mouse) {
		m_mouse_pos = { x, y };
		m_first_mouse = false;
	}

	// GLFW can send multiple cursor events in one frame, especially for high polling-rate mice
	// So we add to the mouse delta
	m_mouse_delta += glm::vec2(x - m_mouse_pos.x, m_mouse_pos.y - y);

	m_mouse_pos = { x, y };
}

}