#pragma once

#include "IPhysicsEngine.h"
#include "Detector.h"
#include "PhysicsSolver.h"

#include "Collision.h"
#include "PhysicsBody.h"


#define MAX_COLLISIONS 64

// Physics can be greatly improved by identifying 'collisions'
// that occur between two bodies and treating them as a single contact

class PhysicsEngine : public IPhysicsEngine {
public:
	vec3 gravity = vec3(0, 0, -1.5f);
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

		biasSlop = 0.01f;
		biasFactor = 0.1f;

		elasticity = 0.f;
		friction = 0.55f;
	}

	void update(float deltaTime);

	virtual void addCollision(Collision collision) override { if (collisionCount >= maxCollisions) return; collisions[collisionCount++] = collision; }
	virtual void clearCollisions() override { collisionCount = 0; }
};