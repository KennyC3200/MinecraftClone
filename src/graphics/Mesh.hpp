#pragma once

#include "graphics/BufferObj.hpp"
#include "graphics/VertexArrayObj.hpp"

#include <glm/glm.hpp>

#include <cstddef>
#include <cstdint>
#include <vector>

namespace mcc {

struct Vertex {
	glm::vec3 m_pos; 	// location 0
	glm::vec2 m_uv;  	// location 1
};

struct MeshData {
	std::vector<Vertex> m_vertices;
	std::vector<std::uint32_t> m_indices;
};

class Mesh final {
public:
	Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;

	// Update VBO and EBO contests. Also update m_indices_cnt for Draw()
	void Upload(const MeshData& data);

	void Draw() const;

private:
	VertexArrayObj m_VAO;
	VertexBufferObj m_VBO;
	ElementBufferObj m_EBO;
	std::size_t m_indices_cnt = 0;
};

}
