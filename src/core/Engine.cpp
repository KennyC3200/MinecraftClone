#include "core/Engine.hpp"
#include "world/ChunkMesher.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace mcc {

Engine::Engine()
    : m_window(1280, 720)
    , m_shader("assets/shaders/test.vert", "assets/shaders/test.frag") 
    , m_block_atlas("assets/textures/blocks.png", GL_RGBA, GL_RGBA)
{
	// Depth
	glEnable(GL_DEPTH_TEST);

	// Face culling
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glFrontFace(GL_CCW);

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

	// Chunk
	for (std::size_t z = 0; z < Chunk::SIZE; z++) {
		for (std::size_t y = 0; y < Chunk::SIZE; y++) {
			for (std::size_t x = 0; x < Chunk::SIZE; x++) {
				m_chunk.SetBlock({x, y, z}, BlockId::Dirt);
			}
		}
	}

	m_chunk_mesh.Upload(BuildChunkMesh(m_chunk));
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
    const glm::vec3 sky_color(0.53f, 0.81f, 0.92f);
    glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 model = glm::mat4(1.0f);
	m_shader.SetUniform("model", model);

	glm::mat4 view = m_cam.ViewMat();
	m_shader.SetUniform("view", view);

	glm::mat4 proj = m_cam.ProjMat((float)m_window.Width() / m_window.Height());
	m_shader.SetUniform("proj", proj);

	m_block_atlas.Bind(0);
	m_shader.Bind();
	m_chunk_mesh.Draw();
}

void Engine::SwapBuffers() { 
	glfwSwapBuffers(m_window.Handle()); 
}

}