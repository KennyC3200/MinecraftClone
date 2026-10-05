#include "world/Chunk.hpp"
#include "world/Block.hpp"

#include <glm/gtx/string_cast.hpp>

#include <stdexcept>

namespace mcc {

namespace {

bool InBounds(const glm::ivec3& pos) {
	return 
		pos.x >= 0 && pos.x < Chunk::SIZE && 
		pos.y >= 0 && pos.y < Chunk::SIZE && 
		pos.z >= 0 && pos.z < Chunk::SIZE;
}

std::size_t Index(const glm::ivec3& pos) {
	return pos.x + Chunk::SIZE * (pos.y + Chunk::SIZE * pos.z);
}

}

BlockId Chunk::GetBlock(const glm::ivec3& pos) const {
	if (!InBounds(pos)) {
		throw std::runtime_error("Chunk::GetBlock out of bounds: " + glm::to_string(pos));
	}

	return m_blocks[Index(pos)];
}

void Chunk::SetBlock(const glm::ivec3& pos, BlockId id) {
	if (!InBounds(pos)) {
		throw std::runtime_error("Chunk::SetBlock out of bounds: " + glm::to_string(pos));
	}

	std::size_t idx = Index(pos);

	if (IsSolid(m_blocks[idx])) m_solid_cnt--;

	m_blocks[idx] = id;
	if(IsSolid(id)) m_solid_cnt++;
}

}
