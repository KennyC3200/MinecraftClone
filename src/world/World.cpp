#include "World.hpp"

namespace mcc {

World::World(int render_dist)
	: m_render_dist(render_dist)
{
	Update({ 0, 0, 0 });
}

Chunk* World::GetChunk(const glm::ivec3& chunk_pos) {
	// `it` returns an iterator that contains the ivec3 and unique_ptr
	// So use it->second to access the unique_ptr
	auto it = m_chunks.find(chunk_pos);
	return it != m_chunks.end() ? it->second.get() : nullptr;
}

const Chunk* World::GetChunk(const glm::ivec3& chunk_pos) const {
	// `it` returns an iterator that contains the ivec3 and unique_ptr
	// So use it->second to access the unique_ptr
	auto it = m_chunks.find(chunk_pos);
	return it != m_chunks.end() ? it->second.get() : nullptr;
}

BlockId World::GetBlock(const glm::ivec3& world_pos) const {
	glm::ivec3 chunk_pos = world_pos >> 4; // Equivalent to floor of divide by 16
	glm::ivec3 local_pos = world_pos & 15; // Get last 4 bits -> chunk local position for the block

	const Chunk* chunk = GetChunk(chunk_pos);

	// If chunk is not loaded yet, return Air
	if (chunk == nullptr) return BlockId::Air;

	return chunk->GetBlock(local_pos);
}

void World::SetBlock(const glm::ivec3& world_pos, BlockId id) {
	glm::ivec3 chunk_pos = world_pos >> 4; // Equivalent to floor of divide by 16
	glm::ivec3 local_pos = world_pos & 15; // Get last 4 bits -> chunk local position for the block

	Chunk* chunk = GetChunk(chunk_pos);

	// If the chunk is not loaded yet, nothing happens
	if (chunk == nullptr) return;

	chunk->SetBlock(local_pos, id);
}

void World::Update(glm::ivec3 center_chunk_pos) {
	for (int x = -m_render_dist; x <= m_render_dist; x++) {
		for (int y = -8; y < 0; y++) {
			for (int z = -m_render_dist; z <= m_render_dist; z++) {
				GenerateChunk(center_chunk_pos + glm::ivec3(x, y, z));
			}
		}
	}
}

// TODO: Add perlin noise to this
void World::GenerateChunk(const glm::ivec3& chunk_pos) {
	auto chunk = std::make_unique<Chunk>();

	// Set all the blocks in chunk to dirt
	for (int k = 0; k < Chunk::SIZE; k++) {
		for (int j = 0; j < Chunk::SIZE; j++) {
			for (int i = 0; i < Chunk::SIZE; i++) {
				chunk.get()->SetBlock({i, j, k}, BlockId::Grass);
			}
		}
	}

	// Move the ownership of the unique_ptr
	m_chunks[chunk_pos] = std::move(chunk);
}

}