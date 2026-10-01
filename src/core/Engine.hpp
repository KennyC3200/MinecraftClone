#pragma once

#include "platform/GLFWContext.hpp"
#include "platform/Window.hpp"
#include "platform/Input.hpp"
#include "graphics/Shader.hpp"
#include "graphics/Texture.hpp"
#include "graphics/Camera.hpp"
#include "graphics/Mesh.hpp"
#include "world/Chunk.hpp"

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
	} m_flags;

    GLFWContext m_context;
    Window m_window;

	Input m_input;

	Camera m_cam;
	float m_cam_speed = 0.0f;

	// Chunks
	Chunk m_chunk;
	Mesh m_chunk_mesh;

    Shader m_shader;
	Texture m_block_atlas;

	// Time
	float m_delta_time = 0.0f;
	float m_last_frame = 0.0f;
};

}