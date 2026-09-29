#include "graphics/VertexArrayObj.hpp"

#include <utility>

namespace mcc {

VertexArrayObj::VertexArrayObj() {
    glGenVertexArrays(1, &m_handle);
}

VertexArrayObj::~VertexArrayObj() {
    if (m_handle > 0) {
        glDeleteVertexArrays(1, &m_handle);
    }
}

VertexArrayObj::VertexArrayObj(VertexArrayObj&& other) noexcept 
    : m_handle(std::exchange(other.m_handle, 0)) {}

VertexArrayObj& VertexArrayObj::operator=(VertexArrayObj&& other) noexcept {
    std::swap(m_handle, other.m_handle);
    return *this;
}

void VertexArrayObj::Bind() const {
    glBindVertexArray(m_handle);
}

void VertexArrayObj::SetAttribPtr(
    const VertexBufferObj& VBO, GLuint idx, GLint components, 
    GLenum type, GLsizei stride, std::size_t offset) 
{
    Bind();
    VBO.Bind();
    glVertexAttribPointer(idx, components, type, GL_FALSE, stride, (void*)(0 + offset));
    glEnableVertexAttribArray(idx);
}

}
