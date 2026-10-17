#pragma once

#include "platform/GLFWContext.hpp"
#include "platform/Window.hpp"
#include "platform/Input.hpp"
#include "graphics/Camera.hpp"
#include "world/World.hpp"
#include "world/WorldRenderer.hpp"
#include "core/ThreadPool.hpp"

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

	struct {
		bool m_wireframe = false;
		int m_render_distance = 64;
	} m_flags;

	GLFWContext m_context;
    Window m_window;

	World m_world;
	WorldRenderer m_world_renderer;

	Input m_input;

	Camera m_cam;
	float m_cam_speed = 0.0f;

	// Time
	float m_delta_time = 0.0f;
	float m_last_frame = 0.0f;

	// ThreadPool needs to be destroyed first
	ThreadPool m_thread_pool;
};

}