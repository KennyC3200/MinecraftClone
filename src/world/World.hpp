#pragma once

#include "world/Block.hpp"
#include "world/Chunk.hpp"
#include "core/ThreadPool.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <memory>
#include <optional>
#include <vector>
#include <array>
#include <unordered_set>

namespace mcc {

inline constexpr int MIN_CHUNK_Y = 0;
inline constexpr int MAX_CHUNK_Y = 8;
inline constexpr int COL_CHUNKS_CNT = MAX_CHUNK_Y - MIN_CHUNK_Y;

struct ChunkChanges {
	std::vector<glm::ivec3> m_loaded;
	std::vector<glm::ivec3> m_unloaded;
};

class World final {
public:
	using MapType = std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>>;

	World(int render_dist);

	World(const World&) = delete;
	World& operator=(const World&) = delete;

	World(World&&) noexcept = delete;
	World& operator=(World&&) noexcept = delete;

	glm::ivec3 GetCenter() const { return m_center ? *m_center : glm::ivec3(0, 0, 0); }

	// Returns nullptr if chunk is not yet loaded
	Chunk* GetChunk(const glm::ivec3& chunk_pos);
	const Chunk* GetChunk(const glm::ivec3& chunk_pos) const;
	static_assert(Chunk::SIZE == 16, "Chunk::SIZE must be 16 for operations in GetChunk");

	BlockId GetBlock(const glm::ivec3& world_pos) const;
	void SetBlock(const glm::ivec3& world_pos, BlockId id);

	int GetRenderDist() const { return m_render_dist; }

	// Returns the chunk changes for ONE FRAME
	ChunkChanges Update(const glm::vec3& pos, ThreadPool& thread_pool);

	const MapType& GetChunks() const { return m_chunks; }

private:
	bool ChunkColInBounds(const glm::ivec2& chunk_col_pos, const glm::ivec2& center_col_pos);
	void UnloadFarChunks(const glm::ivec3& center, ChunkChanges& changes);
	void RebuildPending(const glm::ivec3& center);

	MapType m_chunks;
	std::optional<glm::ivec3> m_center;
	int m_render_dist;

	// Queue for loading chunks
	std::vector<glm::ivec2> m_pending;

	// Multithreading generation for pending chunks
	void GeneratePending(ThreadPool& thread_pool);

	using ChunkCol = std::array<std::unique_ptr<Chunk>, COL_CHUNKS_CNT>;

	struct InFlightChunkCol {
		InFlightChunkCol(const glm::ivec2& col, std::future<ChunkCol> chunks)
			: m_col(col), m_chunks(std::move(chunks)) {}

		glm::ivec2 m_col;
		std::future<ChunkCol> m_chunks;
	};

	std::vector<InFlightChunkCol> m_in_flight_chunk_cols;
	std::unordered_set<glm::ivec2> m_chunk_col_in_flight;
};

}