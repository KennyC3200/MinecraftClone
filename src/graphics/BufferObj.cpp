#include "graphics/BufferObj.hpp"

#include <utility>

namespace mcc {

template <GLenum Target>
BufferObj<Target>::BufferObj() {
    glGenBuffers(1, &m_handle);
}

template <GLenum Target>
BufferObj<Target>::~BufferObj() {
    if (m_handle > 0) {
        glDeleteBuffers(1, &m_handle);
    }
}

template <GLenum Target>
BufferObj<Target>::BufferObj(BufferObj&& other) noexcept
    : m_handle(std::exchange(other.m_handle, 0)) {}

template <GLenum Target>
BufferObj<Target>& BufferObj<Target>::operator=(BufferObj&& other) noexcept {
    std::swap(m_handle, other.m_handle);
    return *this;
}

template <GLenum Target>
void BufferObj<Target>::Bind() const {
    glBindBuffer(Target, m_handle);
}

template class BufferObj<GL_ARRAY_BUFFER>;
template class BufferObj<GL_ELEMENT_ARRAY_BUFFER>;

}
