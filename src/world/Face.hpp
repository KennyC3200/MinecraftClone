#pragma once

#include <glm/glm.hpp>

#include <array>
#include <cstddef>
#include <cstdint>

namespace mcc {

enum class Face : std::uint8_t {
	North, // -z
	South, // +z
	East,  // +x
	West,  // -x
	Up,    // +y
	Down,  // -y
	Count
};

// One face of the unit cube that spans 0 to 1 on each axis.
struct FaceDef {
	// Points out of the block, toward the neighbour that can hide this face.
	glm::ivec3 m_normal;

	// Bottom-left, bottom-right, top-left, top-right, as seen from OUTSIDE the block
	// Makes every face wind counter-clockwise from outside (the side GL_CULL_FACE keeps)
	std::array<glm::vec3, 4> m_corners;
};

inline constexpr auto FACES = std::to_array<FaceDef>({
	// North
	{
		.m_normal = glm::ivec3(0, 0, -1),
		.m_corners = {
			glm::vec3(1, 0, 0),
			glm::vec3(0, 0, 0),
			glm::vec3(1, 1, 0),
			glm::vec3(0, 1, 0)
		}
	},

	// South
	{
		.m_normal = glm::ivec3(0, 0, 1),
		.m_corners = {
			glm::vec3(0, 0, 1),
			glm::vec3(1, 0, 1),
			glm::vec3(0, 1, 1),
			glm::vec3(1, 1, 1)
		}
	},

	// East
	{
		.m_normal = glm::ivec3(1, 0, 0),
		.m_corners = {
			glm::vec3(1, 0, 1),
			glm::vec3(1, 0, 0),
			glm::vec3(1, 1, 1),
			glm::vec3(1, 1, 0)
		}
	},

	// West
	{
		.m_normal = glm::ivec3(-1, 0, 0),
		.m_corners = {
			glm::vec3(0, 0, 0),
			glm::vec3(0, 0, 1),
			glm::vec3(0, 1, 0),
			glm::vec3(0, 1, 1)
		}
	},

	// Up
	{
		.m_normal = glm::ivec3(0, 1, 0),
		.m_corners = {
			glm::vec3(0, 1, 1),
			glm::vec3(1, 1, 1),
			glm::vec3(0, 1, 0),
			glm::vec3(1, 1, 0)
		}
	},

	// Down
	{
		.m_normal = glm::ivec3(0, -1, 0),
		.m_corners = {
			glm::vec3(0, 0, 0),
			glm::vec3(1, 0, 0),
			glm::vec3(0, 0, 1),
			glm::vec3(1, 0, 1)
		}
	},
});

static_assert(
	FACES.size() == static_cast<std::size_t>(Face::Count), "FACES needs one entry per Face");

}
