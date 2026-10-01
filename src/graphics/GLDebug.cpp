#include "graphics/GLDebug.hpp"

#include <iostream>

namespace mcc {

namespace {

const char* ErrorName(GLenum error) {
	switch (error) {
	case GL_INVALID_ENUM:
		return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:
		return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:
		return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION:
		return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:
		return "GL_OUT_OF_MEMORY";
	case GL_STACK_OVERFLOW:
		return "GL_STACK_OVERFLOW";
	case GL_STACK_UNDERFLOW:
		return "GL_STACK_UNDERFLOW";
	default:
		return "UNKNOWN ERROR";
	}
}

const char* SeverityName(GLenum severity) {
	switch (severity) {
	case GL_DEBUG_SEVERITY_HIGH:
		return "HIGH";
	case GL_DEBUG_SEVERITY_MEDIUM:
		return "MEDIUM";
	case GL_DEBUG_SEVERITY_LOW:
		return "LOW";
	case GL_DEBUG_SEVERITY_NOTIFICATION:
		return "NOTIFICATION";
	default:
		return "UNKNOWN SEVERITY";
	}
}

const char* TypeName(GLenum type) {
	switch (type) {
	case GL_DEBUG_TYPE_ERROR:
		return "ERROR";
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
		return "DEPRECATED BEHAVIOR";
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
		return "UNDEFINED BEHAVIOR";
	case GL_DEBUG_TYPE_PORTABILITY:
		return "PORTABILITY";
	case GL_DEBUG_TYPE_PERFORMANCE:
		return "PERFORMANCE";
	case GL_DEBUG_TYPE_MARKER:
		return "MARKER";
	case GL_DEBUG_TYPE_PUSH_GROUP:
		return "PUSH GROUP";
	case GL_DEBUG_TYPE_POP_GROUP:
		return "POP GROUP";
	case GL_DEBUG_TYPE_OTHER:
		return "OTHER";
	default:
		return "UNKNOWN TYPE";
	}
}

}

void EnableGLDebugOutput() {
	if (GLAD_GL_KHR_debug) {
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(OnGLDebugMessage, nullptr);

		// Drop the "info" chatter (NVIDIA prints a line for every buffer it allocates)
		glDebugMessageControl(
			GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, nullptr, GL_FALSE);
	}
}

void APIENTRY OnGLDebugMessage(
	GLenum /*src*/, GLenum type, GLuint id, GLenum severity,
	GLsizei /*length*/, const GLchar* message, const void* /*user_param*/) 
{
#ifndef NDEBUG
	std::cerr << "[GL " << SeverityName(severity) << "] " << TypeName(type)
	          << " (id " << id << "): " << message << '\n';
#endif
} 

void CheckGLError(const char* where) {
#ifndef NDEBUG
	if (GLAD_GL_KHR_debug) return;   // the callback already reports errors as they happen
	for (GLenum err; (err = glGetError()) != GL_NO_ERROR;) {
		std::cerr << "[GL ERROR] " << ErrorName(err) << " at " << where << '\n';
	}
#endif
}

}