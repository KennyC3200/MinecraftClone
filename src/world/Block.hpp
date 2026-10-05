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

constexpr bool IsSolid(BlockId id) {
	return BLOCK_INFOS[static_cast<std::size_t>(id)].m_solid;
}

}