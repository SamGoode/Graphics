#pragma once

#include <glm/ext.hpp>
#include <iostream>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::quat;


static void print(vec2 v) { std::cout << "x: " << v.x << ", y: " << v.y << std::endl; }
static void print(vec3 v) { std::cout << "x: " << v.x << ", y: " << v.y << ", z: " << v.z << std::endl; }
static void print(quat q) { std::cout << "w: " << q.w << ", x: " << q.x << ", y: " << q.y << ", z: " << q.z << std::endl; }

static mat4 genViewMatrix(vec3 location, vec3 forward, vec3 worldUp) {
	vec3 zaxis = normalize(-forward);
	vec3 xaxis = normalize(cross(normalize(worldUp), zaxis));
	vec3 yaxis = cross(zaxis, xaxis);

	mat4 translation = glm::identity<mat4>();
	translation[3] -= vec4(location, 0);

	// columns and rows are in reverse here
	mat4 rotation = {
		{xaxis.x, yaxis.x, zaxis.x, 0},
		{xaxis.y, yaxis.y, zaxis.y, 0},
		{xaxis.z, yaxis.z, zaxis.z, 0},
		{0,       0,       0,       1}
	};

	return rotation * translation;
}

static quat deltaRotation(vec3 angularVelocity, float deltaTime) {
	vec3 w = angularVelocity * (deltaTime * 0.5f);
	float theta = length(w);
	if (theta > 0.f) {
		w = w / theta;
	}
	return quat(cos(theta), sin(theta) * w);
}

static quat eulerToQuat(vec3 eulerDegrees) {
	vec3 euler = glm::radians(eulerDegrees);
	quat qx = quat(cos(euler.x / 2), sin(euler.x / 2) * vec3(1, 0, 0));
	quat qy = quat(cos(euler.y / 2), sin(euler.y / 2) * vec3(0, 1, 0));
	quat qz = quat(cos(euler.z / 2), sin(euler.z / 2) * vec3(0, 0, 1));

	return normalize(qx * qy * qz);
}

static mat4 getTransformMatrix(vec3 position, quat rotation, vec3 scale) {
	mat4 scaleMat = glm::identity<mat4>();
	scaleMat *= vec4(scale, 1);

	mat4 rotMat = glm::mat4_cast(rotation);
	mat4 out = rotMat * scaleMat;
	out[3] += vec4(position, 0);

	return out;
}

