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

	float scaling = 1.f;
};

#define PHYSICSBODY_CAP 32

class Application {
protected:
	GLFWwindow* window;
	float runtime;

	Camera camera;
	vec3 worldUp;

	Mouse mouse;

	int bodyCount = 0;
	const int bodyMaxCount = PHYSICSBODY_CAP;
	PhysicsBody* bodies[PHYSICSBODY_CAP];

	Plane ground;

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

	PhysicsBody* addPhysicsBody(PhysicsBody* physicsBody);
};
