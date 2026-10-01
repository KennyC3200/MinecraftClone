#pragma once

#include "graphics/BufferObj.hpp"

#include <glad/glad.h>

#include <cstddef>

namespace mcc {

class VertexArrayObj final {
public:
    VertexArrayObj();
    ~VertexArrayObj();

    VertexArrayObj(const VertexArrayObj&) = delete;
    VertexArrayObj& operator=(const VertexArrayObj&) = delete;

    VertexArrayObj(VertexArrayObj&& other) noexcept;
    VertexArrayObj& operator=(VertexArrayObj&& other) noexcept;

    void Bind() const;

    void SetAttribPtr(
        const VertexBufferObj& VBO, GLuint idx, GLint components, 
        GLenum type, GLsizei stride, std::size_t offset);
	void SetAttribIPtr(
        const VertexBufferObj& VBO, GLuint idx, GLint components, 
        GLenum type, GLsizei stride, std::size_t offset);

  private:
    GLuint m_handle = 0;
};

}
