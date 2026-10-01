#pragma once

#include <glad/glad.h>

namespace mcc {

void EnableGLDebugOutput();
void APIENTRY OnGLDebugMessage(
	GLenum src, GLenum type, GLuint id, GLenum severity,
	GLsizei /*length*/, const GLchar* message, const void* /*user_param*/); 
void CheckGLError(const char* where);

}