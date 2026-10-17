#pragma once

#include <glm/glm.hpp>

namespace mcc {

// Axis-aligned bounding box: a box whose edges line up with the x, y and z axes, stored as its
// lowest and highest corners. It uses doubles, so the "exactly touching" positions that collision
// produces (feet resting on a block top) stay exact even far from the origin
struct AABB {
	glm::dvec3 m_min;
	glm::dvec3 m_max;

	// The unit cube the block at block_pos fills
	static AABB Block(const glm::ivec3& block_pos) {
		glm::dvec3 min(block_pos);
		return { min, min + 1.0 };
	}

	// True if the boxes share some volume. Boxes that only touch don't count
	bool Intersects(const AABB& other) const {
		return m_min.x < other.m_max.x && m_max.x > other.m_min.x
			&& m_min.y < other.m_max.y && m_max.y > other.m_min.y
			&& m_min.z < other.m_max.z && m_max.z > other.m_min.z;
	}
};

}
