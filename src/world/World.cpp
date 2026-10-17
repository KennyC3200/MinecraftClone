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

bool World::ChunkColInBounds(const glm::ivec2& chunk_col_pos) const 
{
	int dist = std::max(
		std::abs(chunk_col_pos.x - m_center->x), 
		std::abs(chunk_col_pos.y - m_center->z));

	return (dist <= m_render_dist + 1);
}

ChunkChanges World::Update(const glm::vec3& pos, ThreadPool& thread_pool, bool generate) {
	glm::ivec3 center = glm::floor(pos / float(Chunk::SIZE));
	center.y = MIN_CHUNK_Y; // Only the x and z coordinates matter

	ChunkChanges changes;

	// On world startup or the player has moved between chunks
	if (!m_center || *m_center != center) {
		std::optional<glm::ivec3> old_center = m_center;
		m_center = center;
		changes.m_center_moved = true;

		// On startup nothing is loaded yet, so there is nothing to unload
		if (old_center) UnloadFarChunks(*old_center, changes);
		RebuildPending();
	}

	// Generate pending tasks (multithreaded)
	if (generate) GeneratePending(thread_pool);

	// Append the changes
	for (auto it = m_in_flight_jobs.begin(); it != m_in_flight_jobs.end();) {
		if (it->m_chunks.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
			it++;
			continue;
		}

		// Insert them into m_chunks and add to changes.m_loaded
		ChunkCol chunks = it->m_chunks.get();

		// If the chunk is in bounds, add the chunk data to changes
		// Otherwise, if the chunk is out of bounds by the time it is loaded, don't
		if (ChunkColInBounds(it->m_col)) {
			for (int y = MIN_CHUNK_Y; y < MAX_CHUNK_Y; y++) {
				glm::ivec3 chunk_pos(it->m_col.x, y, it->m_col.y);
				m_chunks[chunk_pos] = std::move(chunks[y - MIN_CHUNK_Y]);
				changes.m_loaded.push_back(chunk_pos);
			}
		}

		// Remove the chunk column from the hashset of in flight
		m_in_flight_positions.erase(it->m_col);

		// Then, erase it from m_in_flight_jobs
		it = m_in_flight_jobs.erase(it);
	}

	return changes;
}

void World::UnloadFarChunks(const glm::ivec3& old_center, ChunkChanges& changes) {
	// For render distance R:
	// Unload chunks that are beyond R + 1 in x and z dirs of the new center.
	// Every loaded column was within R + 1 of the old center, so only columns in that old square can
	// have left the bounds. Checking them is just integer math, and only the ones that left need
	// hashmap lookups, instead of visiting every loaded chunk
	int bound = m_render_dist + 1;
	for (int x = old_center.x - bound; x <= old_center.x + bound; x++) {
		for (int z = old_center.z - bound; z <= old_center.z + bound; z++) {
			if (ChunkColInBounds({ x, z })) continue;

			for (int y = MIN_CHUNK_Y; y < MAX_CHUNK_Y; y++) {
				glm::ivec3 chunk_pos(x, y, z);
				if (m_chunks.erase(chunk_pos)) {
					changes.m_unloaded.push_back(chunk_pos);
				}
			}
		}
	}
}

void World::RebuildPending() {
	// Clear the pending queue
	m_pending.clear();

	// For render distance R:
	// Load new chunks in columns within R in x and z directions
	for (int x = -m_render_dist; x <= m_render_dist; x++) {
		for (int z = -m_render_dist; z <= m_render_dist; z++) {
			glm::ivec2 col = { m_center->x + x, m_center->z + z };
			if (
				!m_in_flight_positions.contains(col) && 
				!m_chunks.contains({ col.x, MIN_CHUNK_Y, col.y})) 
			{
				m_pending.push_back(col);
			}
		}
	}

	// Sort the chunks
	glm::ivec2 c(m_center->x, m_center->z);
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
	// Keep creating tasks until max number of workers reached or no more pending chunks
	while (m_in_flight_positions.size() < max_in_flight && !m_pending.empty()) {
		glm::ivec2 col = m_pending.back(); 
		m_pending.pop_back();
		m_in_flight_positions.insert(col);

		// Generate a column (multithreaded)
		m_in_flight_jobs.emplace_back(col, thread_pool.Submit([col] {
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