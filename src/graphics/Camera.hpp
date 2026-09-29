#pragma once

#include <glm/glm.hpp>

namespace mcc {

class Camera final {
public:
	static constexpr float ABS_PITCH_CLAMP = 89.0f;

	void Rotate(float d_yaw, float d_pitch);
	void Move(const glm::vec3& offset);

	glm::mat4 ViewMat() const;
	glm::mat4 ProjMat(float aspect) const;
	const glm::vec3& Front() const { return m_front; }
	glm::vec3 Right() const { return glm::normalize(glm::cross(m_front, m_up)); }

private:
	glm::vec3 m_pos{0.0f, 0.0f, 3.0f};
	glm::vec3 m_front{0.0f, 0.0f, -1.0f};
	glm::vec3 m_up{0.0f, 1.0f,  0.0f};
	float m_yaw = -90.0f;
	float m_pitch = 0.0f;
	float m_fov = 45.0f;
};

}