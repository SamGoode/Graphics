#pragma once

#include <glm/ext.hpp>
#include <iostream>

using glm::vec2;
using glm::vec3;
using glm::vec4;
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