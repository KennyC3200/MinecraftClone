#include "Camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

namespace mcc {

Camera::Camera(const glm::vec3& pos) 
	: m_pos(pos) {}

void Camera::Rotate(float d_yaw, float d_pitch) {
	m_yaw += d_yaw;
	m_pitch += d_pitch;
	m_pitch = glm::clamp(m_pitch, -ABS_PITCH_CLAMP, ABS_PITCH_CLAMP);

	glm::vec3 dir = {
		cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch)), 
		sin(glm::radians(m_pitch)),
		sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch))
	};
	m_front = glm::normalize(dir);
}

void Camera::Move(const glm::vec3& offset) {
	m_pos += offset;
}

glm::mat4 Camera::ViewMat() const {
	return glm::lookAt(m_pos, m_pos + m_front, m_up);
}

glm::mat4 Camera::ProjMat(float aspect, float far_z) const {
	return glm::perspective(glm::radians(45.0f), aspect, 0.1f, far_z); 
}

}