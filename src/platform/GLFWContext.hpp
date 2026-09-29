#pragma once

#include <GLFW/glfw3.h>

namespace mcc {

class GLFWContext final {
public:
    GLFWContext();
    ~GLFWContext();
    GLFWContext(const GLFWContext&) = delete;
    GLFWContext& operator=(const GLFWContext&) = delete;
};

}