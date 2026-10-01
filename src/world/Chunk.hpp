#pragma once

#include "world/Block.hpp"

#include <glm/glm.hpp>

#include <array>
#include <cstddef>

namespace mcc {

class Chunk final {
public:
	static constexpr int SIZE = 16;
	static constexpr std::size_t VOLUME = SIZE * SIZE * SIZE;

	BlockId GetBlock(const glm::ivec3& pos) const;
	void SetBlock(const glm::ivec3& pos, BlockId id);

	static bool InBounds(const glm::ivec3& pos);

private:
	static std::size_t Index(const glm::ivec3& pos);

	std::array<BlockId, VOLUME> m_blocks{};
};

}
