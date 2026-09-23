#pragma once

#include <GLFW/glfw3.h>

#include <stdexcept>
#include <iostream>

namespace mcc {

class GLFWContext final {
public:
    GLFWContext();
    ~GLFWContext();
    GLFWContext(const GLFWContext&) = delete;
    GLFWContext& operator=(const GLFWContext&) = delete;
};

}