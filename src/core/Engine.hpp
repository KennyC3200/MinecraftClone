#pragma once

#include "platform/GLFWContext.hpp"
#include "platform/Window.hpp"
#include "platform/Input.hpp"
#include "graphics/Camera.hpp"
#include "world/World.hpp"
#include "world/WorldRenderer.hpp"
#include "core/ThreadPool.hpp"
#include "game/Hotbar.hpp"
#include "game/Player.hpp"
#include "ui/Hud.hpp"

namespace mcc {

class Engine final {
public:
    Engine();
    void Run();

private:
    void Poll();
    void Update();
	PlayerInput ReadPlayerInput() const;
	void InteractWithBlocks();
    void Render();
    void SwapBuffers();
	void OnCursorMove(double x_pos, double y_pos);

	struct {
		bool m_wireframe = false;
		int m_render_distance = 32;
	} m_flags;

	GLFWContext m_context;
    Window m_window;

	// Needs the GL context, so it comes after m_window (and is destroyed before it)
	Hud m_hud;

	World m_world;
	WorldRenderer m_world_renderer;

	Input m_input;

	// The camera sits at the player's eyes, so the player is constructed first
	Player m_player;
	Camera m_cam;

	// How far away (in blocks) a block can be broken or placed
	static constexpr float BLOCK_REACH = 10.0f;

	// Which block right click places (keys 1-9 pick the slot)
	Hotbar m_hotbar;

	// Time
	float m_delta_time = 0.0f;
	float m_last_frame = 0.0f;

	// ThreadPool needs to be destroyed first
	ThreadPool m_thread_pool;
};

}