#pragma once

#include "world/Block.hpp"
#include "world/Chunk.hpp"

#include <glm/glm.hpp>
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <memory>

namespace mcc {

class World final {
public:
	World();

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

private:
	std::unordered_map<glm::ivec3, std::unique_ptr<Chunk>> m_chunks;
};

}