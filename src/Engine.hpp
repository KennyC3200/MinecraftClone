#pragma once

#include "GLFWContext.hpp"
#include "Window.hpp"
#include "Shader.hpp"
#include "BufferObj.hpp"
#include "VertexArrayObj.hpp"
#include "Texture.hpp"

namespace mcc {

class Engine final {
public:
    Engine();
    void Run();

private:
    void Poll();
    void Update();
    void Render();
    void SwapBuffers();
	void OnCursorMove(double x_pos, double y_pos);
	void OnKey(int key, int action);

    GLFWContext m_context;
    Window m_window;

	// Block
    Shader m_shader;
    VertexBufferObj m_VBO;
    ElementBufferObj m_EBO;
    VertexArrayObj m_VAO;
    Texture m_wall_tex;

	// Camera and cursor
	glm::vec3 m_cam_pos = glm::vec3(0.0f, 0.0f,  3.0f);
	glm::vec3 m_cam_front = glm::vec3(0.0f, 0.0f, -1.0f);
	glm::vec3 m_cam_up = glm::vec3(0.0f, 1.0f,  0.0f);
	float m_cam_speed = 0.0f;

	float m_pitch = 0, m_yaw = -90.0f, m_roll = 0;
	glm::vec3 m_dir;

	bool m_enable_cursor = false;

	// Time
	float m_delta_time = 0.0f;
	float m_last_frame = 0.0f;
};

}