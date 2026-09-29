#pragma once

#include <glad/glad.h>

#include <vector>

namespace mcc {

template <GLenum Target>
class BufferObj final {
public:
    BufferObj();
    ~BufferObj();

    BufferObj(const BufferObj& other) = delete;
    BufferObj& operator=(const BufferObj& other) = delete;

    BufferObj(BufferObj&& other) noexcept;
    BufferObj& operator=(BufferObj&& other) noexcept;

    void Bind() const;
    template <typename T> void SetData(const std::vector<T>& data, GLenum usage);

private:
    GLuint m_handle = 0;

    static_assert(Target == GL_ARRAY_BUFFER || Target == GL_ELEMENT_ARRAY_BUFFER,
    "BufferObj: add an explicit instantiation in BufferObj.cpp for this target type");

};

template <GLenum Target>
template <typename T>
void BufferObj<Target>::SetData(const std::vector<T>& data, GLenum usage) {
    Bind();
    glBufferData(Target, data.size() * sizeof(T), data.data(), usage);
}

// The Vertex Buffer Object is a buffer that stores vertex DATA
// For instance, a vertex can have position data (x, y, z) followed by texture data (u, v)
// One vertex would have representation x, y, z, u, v
// And ofc the vertex buffer object stores all vertices for one specific object
using VertexBufferObj = BufferObj<GL_ARRAY_BUFFER>;

// The Element Buffer Object is a buffer that stores INDICES related to the vertices
// These indices determine in what order the vertices should be drawn
// For instance, for vertices v0 v1 v2, making a triangle, you could set the order of the
// vertices to be drawn as 2, 0, 1, corresponding to drawing in order v2 -> v0 -> v1
using ElementBufferObj = BufferObj<GL_ELEMENT_ARRAY_BUFFER>;

}