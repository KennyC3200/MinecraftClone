#include "world/Chunk.hpp"
#include "world/Block.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>

#include <stdexcept>

namespace mcc {

BlockId Chunk::GetBlock(const glm::ivec3& pos) const {
	// Return Air for now so that faces on the border of chunks will draw
	if (!InBounds(pos)) {
		return BlockId::Air;
	}

	// Add this later when we check neighbouring chunks
	// if (!InBounds(pos)) {
	// 	throw std::runtime_error("Chunk::GetBlock out of bounds: " + glm::to_string(pos));
	// }

	return m_blocks[Index(pos)];
}

void Chunk::SetBlock(const glm::ivec3& pos, BlockId id) {
	if (!InBounds(pos)) {
		throw std::runtime_error("Chunk::SetBlock out of bounds: " + glm::to_string(pos));
	}

	m_blocks[Index(pos)] = id;
}

bool Chunk::InBounds(const glm::ivec3& pos) {
	return 
		pos.x >= 0 && pos.x < SIZE && 
		pos.y >= 0 && pos.y < SIZE && 
		pos.z >= 0 && pos.z < SIZE;
}

std::size_t Chunk::Index(const glm::ivec3& pos) {
	return pos.x + SIZE * (pos.y + SIZE * pos.z);
}

}
