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

bool World::ChunkColInBounds(const glm::ivec2& chunk_col_pos, const glm::ivec2& center_col_pos) {
	int dist = std::max(
		std::abs(chunk_col_pos.x - center_col_pos.x), 
		std::abs(chunk_col_pos.y - center_col_pos.y));

	return (dist <= m_render_dist + 1);
}

ChunkChanges World::Update(const glm::vec3& pos, ThreadPool& thread_pool) {
	glm::ivec3 center = glm::floor(pos / float(Chunk::SIZE));
	center.y = MIN_CHUNK_Y; // Only the x and z coordinates matter

	ChunkChanges changes;

	// On world startup or the player has moved between chunks
	if (!m_center || *m_center != center) {
		m_center = center;
		UnloadFarChunks(center, changes);
		RebuildPending(center);
	}

	// Generate pending tasks (this is multithreaded)
	GeneratePending(thread_pool);

	// Append the changes
	for (auto it = m_in_flight_chunk_cols.begin(); it != m_in_flight_chunk_cols.end();) {
		if (it->m_chunks.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
			it++;
			continue;
		}

		// Insert them into m_chunks and add to changes.m_loaded
		ChunkCol chunks = it->m_chunks.get();

		// Remove the chunk column from the hashset of in flight
		m_chunk_col_in_flight.erase(it->m_col);

		// If the chunk is in bounds, add the chunk data to changes
		// Otherwise, if the chunk is out of bounds by the time it is loaded, don't
		if (ChunkColInBounds(it->m_col, { center.x, center.z })) {
			for (int y = MIN_CHUNK_Y; y < MAX_CHUNK_Y; y++) {
				glm::ivec3 chunk_pos(it->m_col.x, y, it->m_col.y);
				m_chunks[chunk_pos] = std::move(chunks[y - MIN_CHUNK_Y]);
				changes.m_loaded.push_back(chunk_pos);
			}
		}

		// Then, erase it from m_in_flight_chunk_cols
		it = m_in_flight_chunk_cols.erase(it);
	}

	return changes;
}

void World::UnloadFarChunks(const glm::ivec3& center, ChunkChanges& changes) {
	// For render distance R:
	// Find chunks that need to be unloaded. These are chunks that are beyond R + 1 in x and z dirs
	for (auto it = m_chunks.begin(); it != m_chunks.end();) {
		const glm::ivec3& chunk_pos = it->first;
		const glm::ivec2 col = { chunk_pos.x, chunk_pos.z };
		if (!ChunkColInBounds(col, { center.x, center.z })) {
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
			if (
				!m_chunk_col_in_flight.contains(col) && 
				!m_chunks.contains({ col.x, MIN_CHUNK_Y, col.y})) 
			{
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

void World::GeneratePending(ThreadPool& thread_pool) {
	const std::size_t max_in_flight = 2 * thread_pool.Size();

	// Create a task for the generation of each column
	while (m_chunk_col_in_flight.size() < max_in_flight && !m_pending.empty()) {
		glm::ivec2 col = m_pending.back(); 
		m_pending.pop_back();
		m_chunk_col_in_flight.insert(col);

		// Task to generate a column
		m_in_flight_chunk_cols.emplace_back(col, thread_pool.Submit([col] {
			ChunkCol chunks;
			for (int y = MIN_CHUNK_Y; y < MAX_CHUNK_Y; y++) {
				glm::ivec3 chunk_pos = { col.x, y, col.y };
				chunks[y - MIN_CHUNK_Y] = GenerateChunk(chunk_pos);
			}

			return chunks;
		}));
	}
}

}