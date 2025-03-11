#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>

#include "Gizmos.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;

static mat4 genViewMatrix(vec3 location, vec3 forward, vec3 worldUp);

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


class Application {
public:
	bool running;

protected:
	GLFWwindow* window;
	float runtime;

	Camera camera;
	vec3 worldUp;

	float mouseX = 0.f;
	float mouseY = 0.f;

	float rotPitch = 0.f;
	float rotYaw = 0.f;

public:
	Application();

	bool startup(int windowWidth, int windowHeight);
	bool update();
	void draw();
	void shutdown();

	void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
};