#pragma once

#include <glm/glm.hpp>

using glm::vec3;
using glm::quat;

class Camera {
public:
	vec3 pos;
	quat orientation = quat(1, 0, 0, 0);

	float movementSpeed;

public:
	Camera() {}
	Camera(vec3 _pos, vec3 _eulerRotation, float _movementSpeed) {
		pos = _pos;

		vec3 euler = glm::radians(_eulerRotation);
		quat qx = quat(cos(euler.x / 2), sin(euler.x / 2) * vec3(1, 0, 0));
		quat qy = quat(cos(euler.y / 2), sin(euler.y / 2) * vec3(0, 1, 0));
		quat qz = quat(cos(euler.z / 2), sin(euler.z / 2) * vec3(0, 0, 1));
		orientation = orientation * qy;
		orientation = qz * orientation;
		orientation = orientation * qx;

		movementSpeed = _movementSpeed;
	}
};
