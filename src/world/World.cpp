#include "World.hpp"
#include "Terrain.hpp"
#include "world/World.hpp"

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
	center.y = MIN_CHUNK_Y; // Only the x and z coordinates matter

	ChunkChanges changes;

	// On world startup or the player has moved between chunks
	if (!m_center || *m_center != center) {
		m_center = center;
		UnloadFarChunks(center, changes);
		RebuildPending(center);
	}

	GeneratePending(changes);

	return changes;
}

void World::UnloadFarChunks(const glm::ivec3& center, ChunkChanges& changes) {
	// For render distance R:
	// Find chunks that need to be unloaded. These are chunks that are beyond R + 1 in x and z dirs
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
}

void World::RebuildPending(const glm::ivec3& center) {
	// Clear the pending queue
	m_pending.clear();

	// For render distance R:
	// Load new chunks in columns within R in x and z directions
	for (int x = -m_render_dist; x <= m_render_dist; x++) {
		for (int z = -m_render_dist; z <= m_render_dist; z++) {
			glm::ivec2 col = { center.x + x, center.z + z };
			if (!m_chunks.contains({ col.x, MIN_CHUNK_Y, col.y })) {
				m_pending.push_back(col);
			}
		}
	}

	// Sort the chunks
	glm::ivec2 c(center.x, center.z);
	auto dist_squared = [&](const glm::ivec2 p) {
		glm::ivec2 d = p - c;
		return d.x * d.x + d.y * d.y;
	};

	std::sort(
		m_pending.begin(), m_pending.end(),
		[&](const glm::ivec2& a, const glm::ivec2& b) {
			return dist_squared(a) > dist_squared(b);
		}
	);
}

void World::GeneratePending(ChunkChanges& changes) {
	// Load data for 4 columns per frame
	for (int i = 0; i < 4 && !m_pending.empty(); i++) {
		glm::ivec2 col = m_pending.back(); 
		m_pending.pop_back();
		for (int y = MIN_CHUNK_Y; y < MAX_CHUNK_Y; y++) {
			glm::ivec3 chunk_pos = { col.x, y, col.y };
			m_chunks[chunk_pos] = GenerateChunk(chunk_pos);
			changes.m_loaded.push_back(chunk_pos);
		}
	}
}

}