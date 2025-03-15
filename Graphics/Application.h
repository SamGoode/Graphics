#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>

#include "Gizmos.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Camera.h"
#include "Ball.h"
#include "PhysicsBody.h"
#include "Collision.h"

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

	float scaling = 0.001f;
};

#define PHYSICSBODY_CAP 32

class Application {
protected:
	GLFWwindow* window;
	float runtime;

	Camera camera;
	vec3 forward = vec3(1, 0, 0);
	vec3 right = vec3(0, 1, 0);
	vec3 worldUp;

	Mouse mouse;

	int bodyCount = 0;
	const int bodyMaxCount = PHYSICSBODY_CAP;
	PhysicsBody* bodies[PHYSICSBODY_CAP];

	int collisionCount = 0;
	const int maxCollisions = 32;
	Collision collisions[32];

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
	void addCollision(Collision collision);
	void clearCollisions();
};
