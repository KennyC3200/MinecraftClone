#pragma once

#include "world/Face.hpp"

#include <cstdint>

namespace mcc {

enum class BlockId : std::uint8_t {
	Air,
	Dirt,
	Grass,
	Stone,
	Count
};

struct BlockInfo {
	bool m_solid = false;
	int m_tile_top = -1;
	int m_tile_side = -1;
	int m_tile_bottom = -1;

	int TileFor(Face face) const;
};

BlockInfo GetBlockInfo(BlockId id);

}