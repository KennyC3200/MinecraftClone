#include "WorldRenderer.hpp"
#include "ChunkMesher.hpp"

namespace mcc {

WorldRenderer::WorldRenderer() 
	: m_shader("assets/shaders/chunk.vert", "assets/shaders/chunk.frag")
	, m_block_atlas("assets/textures/blocks.png", GL_RGBA, GL_RGBA) {}

Mesh* WorldRenderer::GetChunkMesh(const glm::ivec3& chunk_pos) {
	// `it` returns an iterator that contains the ivec3 and unique_ptr
	// So use it->second to access the unique_ptr
	auto it = m_chunk_meshes.find(chunk_pos);
	return it != m_chunk_meshes.end() ? it->second.get() : nullptr;
}

const Mesh* WorldRenderer::GetChunkMesh(const glm::ivec3& chunk_pos) const {
	// `it` returns an iterator that contains the ivec3 and unique_ptr
	// So use it->second to access the unique_ptr
	auto it = m_chunk_meshes.find(chunk_pos);
	return it != m_chunk_meshes.end() ? it->second.get() : nullptr;
}

void WorldRenderer::RenderWorld(
	const glm::mat4& model, 
	const glm::mat4& view, 
	const glm::mat4 proj) 
{
	m_block_atlas.Bind(0);
	m_shader.Bind();
	m_shader.SetUniform("model", model);
	m_shader.SetUniform("view", view);
	m_shader.SetUniform("proj", proj);

	for (auto& [chunk_pos, mesh] : m_chunk_meshes) {
		if (mesh) {
			m_shader.SetUniform("chunk_origin", chunk_pos * Chunk::SIZE);
			GetChunkMesh(chunk_pos)->Draw();
		}
	}
}

void WorldRenderer::MeshChunk(const World& world, const glm::ivec3& chunk_pos) {
	// Ensure the chunk data has been loaded
	if (!world.GetChunk(chunk_pos)) return;

	PaddedChunk padded_chunk;
	glm::ivec3 origin = chunk_pos * Chunk::SIZE;
	for (int z = -1; z < PaddedChunk::SIZE - 1; z++) {
		for (int y = -1; y < PaddedChunk::SIZE - 1; y++) {
			for (int x = -1; x < PaddedChunk::SIZE - 1; x++) {
				glm::ivec3 pos = { x, y, z };
				padded_chunk.SetBlock(pos, world.GetBlock(origin + pos));
			}
		}
	}

	MeshData mesh_data = BuildChunkMesh(padded_chunk);

	// Upload the mesh data to the mesh
	std::unique_ptr<Mesh>& mesh = m_chunk_meshes[chunk_pos];
	if (!mesh) mesh = std::make_unique<Mesh>();
	mesh->Upload(mesh_data);
}

}