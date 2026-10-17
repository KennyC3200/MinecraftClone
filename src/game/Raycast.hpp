#pragma once

#include "world/World.hpp"

#include <glm/glm.hpp>

#include <optional>

namespace mcc {

struct RaycastHit {
	// The solid block the ray hit
	glm::ivec3 m_block_pos;

	// Normal of the face the ray entered through. m_block_pos + m_normal is the empty cell in front
	// of that face, which is where a placed block goes
	glm::ivec3 m_normal;
};

// Walks the ray from origin along dir one block at a time (Amanatides & Woo voxel traversal) and
// returns the first solid block within max_dist blocks, or nullopt if there is none.
// The block that contains origin is not tested. Unloaded chunks read as air, so the ray passes
// through them
std::optional<RaycastHit> Raycast(
	const World& world,
	const glm::vec3& origin,
	const glm::vec3& dir,
	float max_dist);

}
