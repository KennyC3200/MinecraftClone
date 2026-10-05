#include "core/Engine.hpp"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <chrono>
#include <iostream>

namespace mcc {

using Clock = std::chrono::steady_clock;
using ms = std::chrono::duration<double, std::milli>;

namespace {

glm::vec3 InitialWorldPos(int render_dist) {
	return glm::vec3(render_dist, 1, render_dist);
}

}

Engine::Engine()
    : m_window(1920, 1080)
	, m_cam(InitialWorldPos(2))
	, m_world(8)
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

	m_cam_speed = 50.0f * m_delta_time;

	// Closing the application
	if (m_input.IsKeyDown(GLFW_KEY_Q)) {
		glfwSetWindowShouldClose(m_window.Handle(), true);
	}

	// Movement
	glm::vec3 front = m_cam.Front();
	if (m_input.IsKeyDown(GLFW_KEY_W)) {
		m_cam.Move(m_cam_speed * glm::vec3(front.x, 0, front.z));
	}
	if (m_input.IsKeyDown(GLFW_KEY_S)) {
		m_cam.Move(-m_cam_speed * glm::vec3(front.x, 0, front.z));
	}
	if (m_input.IsKeyDown(GLFW_KEY_A)) {
		m_cam.Move(-m_cam_speed * m_cam.Right());
	}
	if (m_input.IsKeyDown(GLFW_KEY_D)) {
		m_cam.Move(m_cam_speed * m_cam.Right());
	}
	if (m_input.IsKeyDown(GLFW_KEY_SPACE)) {
		m_cam.Move(m_cam_speed * DirVec(Dir::Up));
	}
	if (m_input.IsKeyDown(GLFW_KEY_LEFT_SHIFT)) {
		m_cam.Move(m_cam_speed * DirVec(Dir::Down));
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

	// World
	auto t0 = Clock::now();
	ChunkChanges changes = m_world.Update(m_cam.Pos());
	auto t1 = Clock::now();
	m_world_renderer.Update(m_world, changes);
	auto t2 = Clock::now();

	if (!changes.m_loaded.empty() || !changes.m_unloaded.empty()) {
		std::cout << "world: " << ms(t1 - t0).count() << " ms ("
			<< changes.m_loaded.size() << " loaded, "
			<< changes.m_unloaded.size() << " unloaded), renderer: "
			<< ms(t2 - t1).count() << " ms\n";
	}
}

void Engine::Render() {
    const glm::vec3 sky_color(0.53f, 0.81f, 0.92f);
    glClearColor(sky_color.r, sky_color.g, sky_color.b, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// MVP matrices into the RenderWorld
	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = m_cam.ViewMat();
	glm::mat4 proj = m_cam.ProjMat(
		(float)m_window.Width() / m_window.Height(), 
		1000.0f
	);
	m_world_renderer.RenderWorld(model, view, proj);
}

void Engine::SwapBuffers() { 
	glfwSwapBuffers(m_window.Handle()); 
}

}