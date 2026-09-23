#pragma once

#include "GLFWContext.hpp"
#include "Window.hpp"

namespace mcc {

class Engine final {
public:
    Engine();
    void Run();

private:
    void Poll();
    void Update(float dt);
    void Render();

    GLFWContext m_context;
    Window m_window;
};

}