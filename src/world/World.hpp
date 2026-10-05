#pragma once

#include "world/Block.hpp"
#include "world/Chunk.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <memory>
#include <optional>
#include <vector>

namespace mcc {

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

	// Returns nullptr if chunk is not yet loaded
	Chunk* GetChunk(const glm::ivec3& chunk_pos);
	const Chunk* GetChunk(const glm::ivec3& chunk_pos) const;
	static_assert(Chunk::SIZE == 16, "Chunk::SIZE must be 16 for operations in GetChunk");

	BlockId GetBlock(const glm::ivec3& world_pos) const;
	void SetBlock(const glm::ivec3& world_pos, BlockId id);

	int GetRenderDist() const { return m_render_dist; }

	ChunkChanges Update(const glm::vec3& pos);

	const MapType& GetChunks() const { return m_chunks; }

private:
	void GenerateChunk(const glm::ivec3& chunk_pos);

	MapType m_chunks;
	std::optional<glm::ivec3> m_center;
	int m_render_dist;
};

}