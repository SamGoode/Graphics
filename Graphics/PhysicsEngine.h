#pragma once

#include "Collision.h"

#define MAX_COLLISIONS 32


class PhysicsEngine {
public:
	class GameEngine* parent = nullptr;

	vec3 gravity = vec3(0, 0, -1.5f);

	int iterations = 2;

	int collisionCount = 0;
	const int maxCollisions = MAX_COLLISIONS;
	Collision collisions[MAX_COLLISIONS];

	typedef void (PhysicsEngine::* func)(PhysicsObject* A, PhysicsObject* B);
	func f[3] = {
		&PhysicsEngine::checkCollision00,
		&PhysicsEngine::checkCollision01,
		&PhysicsEngine::checkCollision11
	};

public:
	PhysicsEngine() {}

	void setParentApp(GameEngine* _parent) { parent = _parent; }

	void update(float deltaTime);

	void solveImpulse(Collision& collision);
	void solveImpulseSingleBody(Collision& collision);

	void checkCollision(PhysicsObject* body, Plane plane);
	void checkCollision(PhysicsObject* A, PhysicsObject* B);

	void checkCollision00(PhysicsObject* boxA, PhysicsObject* boxB);
	void checkCollision01(PhysicsObject* box, PhysicsObject* sphere);
	void checkCollision11(PhysicsObject* sphereA, PhysicsObject* sphereB);

	void addCollision(Collision collision) { if (collisionCount >= maxCollisions) { return; } collisions[collisionCount++] = collision; }
	void clearCollisions() { collisionCount = 0; }
};