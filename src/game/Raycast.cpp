#include "game/Raycast.hpp"

#include <cmath>
#include <limits>

namespace mcc {

namespace {

constexpr float INF = std::numeric_limits<float>::infinity();

// How far along the ray (t) it is from s to the next block boundary on one axis, when s moves by
// ds per unit of t. An axis the ray doesn't move along never reaches a boundary
float IntBound(float s, float ds) {
	if (ds > 0.0f) return (std::floor(s) + 1.0f - s) / ds;
	if (ds < 0.0f) return (s - std::floor(s)) / -ds;
	return INF;
}

}

std::optional<RaycastHit> Raycast(
	const World& world,
	const glm::vec3& origin,
	const glm::vec3& dir,
	float max_dist)
{
	float len = glm::length(dir);
	if (len == 0.0f) return std::nullopt;

	// Normalised, so t (the distance along the ray) is measured in blocks and compares to max_dist
	glm::vec3 d = dir / len;

	glm::ivec3 pos = glm::ivec3(glm::floor(origin));
	glm::ivec3 step = glm::ivec3(glm::sign(d));

	// t at which the ray crosses the next block boundary on each axis
	glm::vec3 t_max(IntBound(origin.x, d.x), IntBound(origin.y, d.y), IntBound(origin.z, d.z));

	// t it takes to cross one whole block on each axis
	glm::vec3 t_delta(
		step.x != 0 ? 1.0f / std::abs(d.x) : INF,
		step.y != 0 ? 1.0f / std::abs(d.y) : INF,
		step.z != 0 ? 1.0f / std::abs(d.z) : INF);

	while (true) {
		// Step along the axis whose boundary comes first
		int axis = t_max.x < t_max.y
			? (t_max.x < t_max.z ? 0 : 2)
			: (t_max.y < t_max.z ? 1 : 2);

		// The next block is already out of reach
		if (t_max[axis] > max_dist) return std::nullopt;

		pos[axis] += step[axis];
		t_max[axis] += t_delta[axis];

		if (IsSolid(world.GetBlock(pos))) {
			glm::ivec3 normal(0);
			normal[axis] = -step[axis];
			return RaycastHit{ pos, normal };
		}
	}
}

}
