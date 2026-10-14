#include "Frustum.hpp"

namespace mcc {

Frustum::Frustum(const glm::mat4& proj, const glm::mat4& view) {
	glm::mat4 m = proj * view;
	glm::vec4 r0 = glm::row(m, 0);
	glm::vec4 r1 = glm::row(m, 1);
	glm::vec4 r2 = glm::row(m, 2);
	glm::vec4 r3 = glm::row(m, 3);

	// Each plane is a vec4. xyz is the normal, pointing into the visible region
	// (each normal vector points into the frustum box)
	// and w is the offset. A point p is on the visible side when dot(xyz, p) + w >= 0
	m_planes = {
		r3 + r0, // left
		r3 - r0, // right
		r3 + r1, // bottom
		r3 - r1, // top
		r3 + r2, // near
		r3 - r2, // far
	};
}

bool Frustum::IntersectsBox(const glm::vec3& min, const glm::vec3& max) const {
	// Test if the AABB for the cube (our block) is outside the frustum box
	for (const glm::vec4& plane : m_planes) {
		glm::vec3 corner = {
			plane.x >= 0 ? max.x : min.x,
			plane.y >= 0 ? max.y : min.y,
			plane.z >= 0 ? max.z : min.z
		};

		// Check if the corner point is on the visible side of the plane
		if (glm::dot(glm::vec3(plane), corner) + plane.w < 0) return false;
	}

	// The cube is inside the frustum box
	return true;
}

}