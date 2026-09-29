#include "core/Engine.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

namespace mcc {

Engine::Engine()
    : m_window(1280, 720)
    , m_shader("assets/shaders/test.vert", "assets/shaders/test.frag") 
    , m_wall_tex("assets/textures/wall.jpg", GL_RGB, GL_RGB) 
{
	// Depth
	glEnable(GL_DEPTH_TEST);

    // Blending function
    glEnable(GL_BLEND);

    // The blend function is src.rgb * X + dst.rgb * B
    // Here, X = GL_SRC_ALPHA = src.a and Y = GL_ONE_MINUS_SRC_ALPHA = 1 - src.a
    // Yielding: src.rgb * src.a + dst.rgb * (1 - src.a)
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// No wireframe
    glPolygonMode(GL_FRONT_AND_BACK, m_flags.m_wireframe ? GL_LINE : GL_FILL);

	// Cursor
	glfwSetInputMode(
		m_window.Handle(), GLFW_CURSOR, 
		m_window.IsCursorCaptured() ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);

	// Assign *this to window void* pointer. This way, you can use static_cast<Engine*> to get the
	// underlying engine object assigned to the GLFWwindow object
	glfwSetWindowUserPointer(m_window.Handle(), this);

	// Camera
	glfwSetCursorPosCallback(m_window.Handle(), [](GLFWwindow* window, double x_pos, double y_pos) {
		static_cast<Engine*>(glfwGetWindowUserPointer(window))->m_input.OnCursorMove(x_pos, y_pos);
	});

	// On key
	glfwSetKeyCallback(m_window.Handle(), [](GLFWwindow* window, int key, int, int action, int) {
		static_cast<Engine*>(glfwGetWindowUserPointer(window))->m_input.OnKey(key, action);
	});
}

void Engine::Run() {
    while (!glfwWindowShouldClose(m_window.Handle())) {
        Poll();
        Update();
        Render();
        SwapBuffers();
    }
}

void Engine::Poll() {
	// Clear the input frame
	m_input.BeginFrame();

	// Process events in the queue and input callbacks 
    glfwPollEvents();
}

void Engine::Update() {
	float current_frame = glfwGetTime();
	m_delta_time = current_frame - m_last_frame;
	m_last_frame = current_frame;

	m_cam_speed = 5.0f * m_delta_time;

	// INPUT
	// ---------------------------------------------------------------------------------------------
	// Closing the application
	if (m_input.IsKeyDown(GLFW_KEY_Q)) {
		glfwSetWindowShouldClose(m_window.Handle(), true);
	}

	// Movement
	if (m_input.IsKeyDown(GLFW_KEY_W)) {
		m_cam.Move(m_cam_speed * m_cam.Front());
	}
	if (m_input.IsKeyDown(GLFW_KEY_S)) {
		m_cam.Move(-m_cam_speed * m_cam.Front());
	}
	if (m_input.IsKeyDown(GLFW_KEY_A)) {
		m_cam.Move(-m_cam_speed * m_cam.Right());
	}
	if (m_input.IsKeyDown(GLFW_KEY_D)) {
		m_cam.Move(m_cam_speed * m_cam.Right());
	}

	// Toggle/untoggle cursor
	if (m_input.IsKeyPressed(GLFW_KEY_ESCAPE)) {
		m_window.SetCursorCaptured(!m_window.IsCursorCaptured());
		glfwSetInputMode(
			m_window.Handle(), GLFW_CURSOR,
			m_window.IsCursorCaptured() ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}

	// Wireframe
	if (m_input.IsKeyPressed(GLFW_KEY_T)) {
		m_flags.m_wireframe = !m_flags.m_wireframe;
		glPolygonMode(GL_FRONT_AND_BACK, m_flags.m_wireframe ? GL_LINE : GL_FILL);
	}

	// Camera
	static const float sens = 0.1f;
	if (m_window.IsCursorCaptured()) {
		glm::vec2 d = m_input.MouseDelta() * sens;
		m_cam.Rotate(d.x, d.y);
	} else {
		m_input.SetFirstMouse(true);
	}
}

void Engine::Render() {
	std::vector<float> vertices = {
        // North (-z)
        0, 0, 0, 	0, 0,
        1, 0, 0, 	1, 0,
        0, 1, 0,	0, 1,
        1, 1, 0,	1, 1,

        // South (+z)
        0, 0, 1,	0, 0,
        1, 0, 1,	1, 0,
        0, 1, 1,	0, 1,
        1, 1, 1,	1, 1,

        // East (+x)
        1, 0, 1,	0, 0,
        1, 0, 0,	1, 0,
        1, 1, 1,	0, 1,
        1, 1, 0,	1, 1,

        // West (-x)
        0, 0, 1,	0, 0,
        0, 0, 0,	1, 0,
        0, 1, 1,	0, 1,
        0, 1, 0,	1, 1,

        // Up (+y)
        0, 1, 1,	0, 0,
        1, 1, 1,	1, 0,
        0, 1, 0,	0, 1,
        1, 1, 0,	1, 1,

        // Down (-y)
        0, 0, 1,	0, 0,
        1, 0, 1,	1, 0,
        0, 0, 0,	0, 1,
        1, 0, 0,	1, 1,
    };

	std::vector<unsigned int> indices;
	for (int i = 0; i < 6; i++) {
		unsigned int offset = i * 4;
		std::vector<unsigned int> _indices = {
			 2 + offset, 0 + offset, 1 + offset,
			 2 + offset, 3 + offset, 1 + offset,
		};
		indices.insert(indices.end(), _indices.begin(), _indices.end());
	}

    m_shader.Bind();

    m_shader.SetUniform("wall_tex", 0);
    m_wall_tex.Bind(0);

    // Attribute pointer
    m_VAO.SetAttribPtr(m_VBO, 0, 3, GL_FLOAT, 5 * sizeof(float), 0);
    m_VAO.SetAttribPtr(m_VBO, 1, 2, GL_FLOAT, 5 * sizeof(float), 3 * sizeof(float));

    // Store the vertices data in the buffer
    m_VBO.SetData(vertices, GL_STATIC_DRAW);

    // Binding the VAO -> Binding the EBO -> EBO is stored on the VAO
    // Therefore, don't need to rebind the EBO each time
    m_EBO.SetData(indices, GL_STATIC_DRAW);

	glm::mat4 model = glm::mat4(1.0f);
	model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	m_shader.SetUniform("model", model);

	glm::mat4 view = m_cam.ViewMat();
	m_shader.SetUniform("view", view);

	glm::mat4 proj = m_cam.ProjMat((float)m_window.Width() / m_window.Height());
	m_shader.SetUniform("proj", proj);

    const glm::vec3 sky_color(0.53f, 0.81f, 0.92f);
    glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}

void Engine::SwapBuffers() { 
	glfwSwapBuffers(m_window.Handle()); 
}

}