#include "WorldRenderer.hpp"
#include "ChunkMesher.hpp"

#include <unordered_set>

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

void WorldRenderer::Update(const World& world, const ChunkChanges& changes) {
	std::unordered_set<glm::ivec3> to_mesh;

	// Erase the unloaded chunks from the meshes
	for (const glm::ivec3& unloaded : changes.m_unloaded) {
		m_chunk_meshes.erase(unloaded);
	}

	// Mesh loaded chunks
	for (const glm::ivec3& loaded : changes.m_loaded) {
		to_mesh.insert(loaded);
		for (const FaceDef& face : FACES) {
			to_mesh.insert(loaded + face.m_normal);
		}
	}

	for (const auto& chunk_pos : to_mesh) {
		MeshChunk(world, chunk_pos);
	}
}

void WorldRenderer::RenderWorld(
	const glm::mat4& model, 
	const glm::mat4& view, 
	const glm::mat4& proj) 
{
	m_block_atlas.Bind(0);
	m_shader.Bind();
	m_shader.SetUniform("model", model);
	m_shader.SetUniform("view", view);
	m_shader.SetUniform("proj", proj);

	for (auto& [chunk_pos, mesh] : m_chunk_meshes) {
		m_shader.SetUniform("chunk_origin", chunk_pos * Chunk::SIZE);
		mesh->Draw();
	}
}

void WorldRenderer::MeshChunk(const World& world, const glm::ivec3& chunk_pos) {
	// Ensure the chunk data has been loaded
	if (!world.GetChunk(chunk_pos)) return;

	PaddedChunk padded_chunk;

	// Each chunk has 26 neighbours + the original chunk in the center = 27 chunks
	// Like a Rubix cube
	std::array<const Chunk*, 27> neighbours{};
	auto index = [](const glm::ivec3& offset) {
		return (offset.x + 1) + 3 * ((offset.y + 1) + 3 * (offset.z + 1));
	};

	// Populate the neighbours array
	for (int z = -1; z <= 1; z++) {
		for (int y = -1; y <= 1; y++) {
			for (int x = -1; x <= 1; x++) {
				glm::ivec3 neighbour(x, y, z);
				neighbours[index(neighbour)] = world.GetChunk(chunk_pos + neighbour);
			}
		}
	}

	const Chunk* center = neighbours[index({0, 0, 0})];

	auto is_full = [&](const glm::ivec3& offset) {
		const Chunk* chunk = neighbours[index(offset)];
		return chunk ? chunk->IsFull() : offset.y <= 0;
	};

	bool buried = center->IsFull();
	for (const FaceDef& face : FACES) {
		buried = buried && is_full(face.m_normal);
	}

	if (center->IsEmpty() || buried) {
		m_chunk_meshes.erase(chunk_pos);
		return;
	}

	// Fill in the blocks for the padded chunk
	// Trick is, given that pos runs from -1 to 16:
	// pos		pos >> 4		pos & 15
	// ---------------------------------
	// -1		-1				15
	// 0..15	0				0..15
	// 16		1				0
	// So pos >> 4 gives the chunk and pos & 15 gives the block inside the chunk
	for (int z = -1; z < PaddedChunk::SIZE - 1; z++) {
		for (int y = -1; y < PaddedChunk::SIZE - 1; y++) {
			for (int x = -1; x < PaddedChunk::SIZE - 1; x++) {
				glm::ivec3 pos(x, y, z);
				glm::ivec3 off = pos >> 4;
				const Chunk* src = neighbours[index(off)];
				padded_chunk.SetBlock(pos, src ? src->GetBlock(pos & 15) 
					: (off.y > 0 ? BlockId::Air : BlockId::Stone)); 
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