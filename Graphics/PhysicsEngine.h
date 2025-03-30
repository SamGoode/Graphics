#pragma once

#include "IPhysicsEngine.h"
#include "Detector.h"
#include "PhysicsSolver.h"

#include "Collision.h"
#include "PhysicsObject.h"


#define MAX_COLLISIONS 64


class PhysicsEngine : public IPhysicsEngine {
public:
	vec3 gravity = vec3(0, 0, -9.8f);
	Plane ground = Plane(vec3(0, 0, 1), 0.f);

private:
	Detector detector = Detector(this);
	PhysicsSolver solver = PhysicsSolver(this);

	int collisionCount = 0;
	const int maxCollisions = MAX_COLLISIONS;
	Collision collisions[MAX_COLLISIONS];

public:
	PhysicsEngine() {
		iterations = 3;

		biasSlop = 0.005f;
		biasFactor = 0.2f;

		elasticity = 0.4f;
		friction = 0.55f;
	}

	void update(float deltaTime);

	virtual void addCollision(Collision collision) override { if (collisionCount >= maxCollisions) return; collisions[collisionCount++] = collision; }
	virtual void clearCollisions() override { collisionCount = 0; }
};