#pragma once

#include "world/Block.hpp"

#include <glm/glm.hpp>

#include <array>
#include <cstddef>

namespace mcc {

class Chunk final {
public:
	Chunk() = default;

	Chunk(const Chunk&) = delete;
	Chunk& operator=(const Chunk&) = delete;

	Chunk(Chunk&& other) noexcept = default;
	Chunk& operator=(Chunk&& other) noexcept = default;

	static constexpr int SIZE = 16;
	static constexpr std::size_t VOLUME = SIZE * SIZE * SIZE;

	BlockId GetBlock(const glm::ivec3& pos) const;
	void SetBlock(const glm::ivec3& pos, BlockId id);

private:

	std::array<BlockId, VOLUME> m_blocks{};
};

}
