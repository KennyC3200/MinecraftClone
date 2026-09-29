#include "platform/GLFWContext.hpp"

#include <iostream>
#include <stdexcept>

namespace mcc {

namespace {

void GLFWErrorCB(int error, const char* description) {
    std::cerr << "GLFW error " << error << ": " << description << '\n';
}

}

GLFWContext::GLFWContext() {
    glfwSetErrorCallback(GLFWErrorCB);
    if (!glfwInit()) {
        throw std::runtime_error("Failed to init GLFW");
    }
}

GLFWContext::~GLFWContext() {
    glfwTerminate();
}

}