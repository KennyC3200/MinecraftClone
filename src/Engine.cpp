#include "Engine.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

namespace mcc {

Engine::Engine()
    : m_window(1280, 720)
{
}

void Engine::Run() {
    // while window is open:
    // measure time since last frame (dt)
    // poll events / process input
    // update(dt)      // player movement, physics, chunk loading
    // render()        // clear, draw world, draw UI
    // swap buffers

    const glm::vec3 skyColor(0.53f, 0.81f, 0.92f);

    while (!glfwWindowShouldClose(m_window.Handle())) {
        glfwPollEvents();

        // QUIT key
        if (glfwGetKey(m_window.Handle(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(m_window.Handle(), GLFW_TRUE);
        }

        glClearColor(skyColor.r, skyColor.g, skyColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glfwSwapBuffers(m_window.Handle());
    }
}

void Engine::Poll() {
}

void Engine::Update(float dt) {
}

void Engine::Render() {
}

}