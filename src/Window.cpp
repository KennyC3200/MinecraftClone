#include "Window.hpp"

#include <glad/glad.h>

#include <iostream>
#include <utility>
#include <stdexcept>

namespace mcc {

namespace {

void FrameBufferSizeCB(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

}

Window::Window(int width, int height)
    : m_width(width), m_height(height)
{
    // OpenGL 4.1 core is the newest version available on macOS.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE); // required on macOS

    m_handle = glfwCreateWindow(
        m_width, m_height, 
        "MinecraftClone v" MC_VERSION,
        nullptr, nullptr);

    if (!m_handle) {
        throw std::runtime_error("Failed to load window");
    }

    glfwMakeContextCurrent(m_handle);
    glfwSwapInterval(1); // vsync

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        glfwDestroyWindow(m_handle);
        throw std::runtime_error("Failed to load OpenGL functions");
    }

    std::cout << "MinecraftClone v" << MC_VERSION << '\n'
              << "OpenGL " << glGetString(GL_VERSION) << '\n'
              << "Renderer " << glGetString(GL_RENDERER) << std::endl;

    // Framebuffer size can differ from window size on high-DPI displays.
    glfwSetFramebufferSizeCallback(m_handle, FrameBufferSizeCB);
    int frame_buffer_w = 0;
    int frame_buffer_h = 0;
    glfwGetFramebufferSize(m_handle, &frame_buffer_w, &frame_buffer_h);
    glViewport(0, 0, frame_buffer_w, frame_buffer_h);
}

Window::~Window() {
    if (m_handle) {
        glfwDestroyWindow(m_handle);
    }
}

Window::Window(Window&& other) noexcept 
    : m_handle(std::exchange(other.m_handle, nullptr))
    , m_width(std::exchange(other.m_width, 0))
    , m_height(std::exchange(other.m_height, 0)) {}

Window& Window::operator=(Window&& other) noexcept {
    std::swap(m_handle, other.m_handle);
    std::swap(m_width, other.m_width);
    std::swap(m_height, other.m_height);
    return *this;
}

}
