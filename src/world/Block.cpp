#include "world/Block.hpp"

#include <array>
#include <cstddef>
#include <stdexcept>
#include <string>

namespace mcc {

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
