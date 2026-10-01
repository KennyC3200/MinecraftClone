#include "world/Block.hpp"

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace mcc {

namespace {

constexpr auto BLOCK_INFOS = std::to_array<BlockInfo>({
	// Air
	{
		.m_solid = false
	},

	// Dirt
	{
		.m_solid = true,
		.m_tile_top = 2,
		.m_tile_side = 2,
		.m_tile_bottom = 2
	},

	// Grass
	{
		.m_solid = true,
		.m_tile_top = 0,
		.m_tile_side = 1,
		.m_tile_bottom = 2
	},

	// Stone
	{
		.m_solid = true,
		.m_tile_top = 3,
		.m_tile_side = 3,
		.m_tile_bottom = 3
	},
});

static_assert(
	BLOCK_INFOS.size() == static_cast<std::size_t>(BlockId::Count),
	"BLOCK_INFOS needs one entry per BlockId");

}

int BlockInfo::TileFor(Face face) const {
	switch (face) {
		case Face::North: 	return m_tile_side;
		case Face::South:	return m_tile_side;
		case Face::East:	return m_tile_side;
		case Face::West:	return m_tile_side;
		case Face::Up:		return m_tile_top;
		case Face::Down:	return m_tile_bottom;
		default: 			return -1;
	}
}

BlockInfo GetBlockInfo(BlockId id) {
	std::size_t arr_id = static_cast<std::size_t>(id);
	if (arr_id >= BLOCK_INFOS.size()) {
		throw std::runtime_error("GetBlockInfo: invalid id" + std::to_string(arr_id));
	}

	return BLOCK_INFOS[arr_id];
}

}
