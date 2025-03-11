#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>

#include "Gizmos.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Camera.h"
#include "Ball.h"
#include "PhysicsBody.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;


static mat4 genViewMatrix(vec3 location, vec3 forward, vec3 worldUp);

static void print(vec2);

struct Mouse {
	vec2 prevPos;
	vec2 pos;
	vec2 input;

	float scaling = 0.3f;
};


class Application {
protected:
	GLFWwindow* window;
	float runtime;

	Camera camera;
	vec3 worldUp;

	Mouse mouse;

	float rotPitch = 0.f;
	float rotYaw = 0.f;

	int bodyCount = 0;
	PhysicsBody* bodies[32];

	//Ball ball = {
	//	.m_radius = 0.5f,
	//	.m_mass = 5.f,
	//	.m_pos = vec3(0, 10, 0),
	//	.m_color = vec4(0.8f, 0, 0, 1)
	//};

public:
	Application();
	~Application() { for (int i = 0; i < bodyCount; i++) { delete bodies[i]; } }

	bool startup(int windowWidth, int windowHeight);
	bool update();
	void draw();
	void shutdown();

	bool shouldClose() { return glfwWindowShouldClose(window); }

	bool keyPressed(int key) { return glfwGetKey(window, key) == GLFW_PRESS; }
	void mouseCursorCallback(GLFWwindow* window, double xpos, double ypos);
};
