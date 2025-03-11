#pragma once

#include <glm/glm.hpp>

using glm::vec3;

class Camera {
public:
	vec3 m_pos;
	vec3 m_forward;

	float m_movementSpeed;

public:
	Camera() {}
	Camera(vec3 pos, vec3 forward, float movementSpeed) {
		m_pos = pos;
		m_forward = forward;

		m_movementSpeed = movementSpeed;
	}
};
