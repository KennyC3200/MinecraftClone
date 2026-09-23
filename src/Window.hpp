#pragma once

#include <GLFW/glfw3.h>

namespace mcc {

class Window final {
public:
    Window(int width, int height);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    Window(Window&& other) noexcept;
    Window& operator=(Window&& other) noexcept;

    [[nodiscard]] GLFWwindow* Handle() const & noexcept { return m_handle; }
    GLFWwindow* Handle() const && = delete;

private:
    GLFWwindow* m_handle = nullptr;
    int m_width = 0;
    int m_height = 0;
};

}