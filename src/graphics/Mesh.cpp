#include "graphics/Mesh.hpp"

#include <glad/glad.h>

#include <utility>

namespace mcc {

Mesh::Mesh() {
	m_VAO.SetAttribIPtr(m_VBO, 0, 1, GL_UNSIGNED_INT, sizeof(Vertex), 0);
}

Mesh::Mesh(Mesh&& other) noexcept 
	: m_VAO(std::move(other.m_VAO)) 
	, m_VBO(std::move(other.m_VBO)) 
	, m_EBO(std::move(other.m_EBO))
	, m_indices_cnt(std::exchange(other.m_indices_cnt, 0)) {}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
	std::swap(m_VAO, other.m_VAO);
	std::swap(m_VBO, other.m_VBO);
	std::swap(m_EBO, other.m_EBO);
	std::swap(m_indices_cnt, other.m_indices_cnt);
	return *this;
}

void Mesh::Upload(const MeshData& data) {
	m_VBO.SetData(data.m_vertices, GL_STATIC_DRAW);

	m_VAO.Bind();

	m_EBO.SetData(data.m_indices, GL_STATIC_DRAW);

	m_indices_cnt = data.m_indices.size();
}

void Mesh::Draw() const {
	m_VAO.Bind();
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices_cnt), GL_UNSIGNED_INT, nullptr);
}

}
