#include "World.hpp"

#include <algorithm>
#include <cstdlib>

namespace mcc {

World::World(int render_dist)
	: m_render_dist(render_dist)
{}

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

ChunkChanges World::Update(const glm::vec3& pos) {
	glm::ivec3 center = glm::floor(pos / float(Chunk::SIZE));
	center.y = 0; // Only the x and z coordinates matter

	// No change
	if (m_center && *m_center == center) return {};

	// For render distance R:
	// Find chunks that need to be unloaded. These are chunks that are beyond R + 1 in x and z
	// directions
	ChunkChanges changes;
	for (auto it = m_chunks.begin(); it != m_chunks.end();) {
		const glm::ivec3& chunk_pos = it->first;
		int dist = std::max(std::abs(chunk_pos.x - center.x), std::abs(chunk_pos.z - center.z));

		if (dist > m_render_dist + 1) {
			changes.m_unloaded.push_back(chunk_pos);
			it = m_chunks.erase(it);
		} else {
			it++;
		}
	}

	// For render distance R:
	// Load new chunks within R in x and z directions
	for (int x = -m_render_dist; x <= m_render_dist; x++) {
		for (int y = -8; y < 0; y++) {
			for (int z = -m_render_dist; z <= m_render_dist; z++) {
				glm::ivec3 chunk_pos = { center.x + x, y, center.z + z };
				if (!m_chunks.contains(chunk_pos)) {
					GenerateChunk(chunk_pos);
					changes.m_loaded.push_back(chunk_pos);
				}
			}
		}
	}

	m_center = center;

	return changes;
}

// TODO: Add perlin noise to this
void World::GenerateChunk(const glm::ivec3& chunk_pos) {
	auto chunk = std::make_unique<Chunk>();

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