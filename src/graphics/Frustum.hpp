#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_access.hpp>

#include <array>

namespace mcc {

class Frustum {
public:
	Frustum(const glm::mat4& proj, const glm::mat4& view);

	bool IntersectsBox(const glm::vec3& min, const glm::vec3& max) const;

private:
	std::array<glm::vec4, 6> m_planes;
};

}