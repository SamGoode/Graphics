#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>

#include "Gizmos.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Camera.h"
#include "Ball.h"

using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;

static mat4 genViewMatrix(vec3 location, vec3 forward, vec3 worldUp);

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

	Ball ball = {
		.m_radius = 0.5f,
		.m_mass = 5.f,
		.m_pos = vec3(0, 10, 0)
	};

public:
	Application();

	bool startup(int windowWidth, int windowHeight);
	bool update();
	void draw();
	void shutdown();

	void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
};