#pragma once

#include "Collision.h"

#define MAX_COLLISIONS 32

class PhysicsEngine {
private:
	class GameEngine* parent = nullptr;

	vec3 gravity = vec3(0, 0, -1.5f);

	int iterations = 1;

	int collisionCount = 0;
	const int maxCollisions = MAX_COLLISIONS;
	Collision collisions[MAX_COLLISIONS];

public:
	PhysicsEngine() {}

	void setParentApp(GameEngine* _parent) { parent = _parent; }

	void update(float deltaTime);

	void solveImpulse(Collision& collision);
	void solveImpulseSingleBody(Collision& collision);

	void checkCollision(PhysicsBody* body, Plane plane);
	//void checkCollision(Sphere* sphere, Plane plane);

	void addCollision(Collision collision) { if (collisionCount >= maxCollisions) { return; } collisions[collisionCount++] = collision; }
	void clearCollisions() { collisionCount = 0; }
};