#pragma once

#include "IPhysicsEngine.h"
#include "PhysicsSolver.h"

#include "Collision.h"
#include "ECS.h"


#define MAX_COLLISIONS 128


class PhysicsEngine : public IPhysicsEngine {
private:
	PhysicsSolver solver = PhysicsSolver(this);

	const int maxTicksPerUpdate = 5;
	const float fixedTimeStep = 0.01f;
	float accumulatedTime = 0.f;

	int collisionCount = 0;
	const int maxCollisions = MAX_COLLISIONS;
	CollisionECS collisionsECS[MAX_COLLISIONS];

public:
	PhysicsEngine() {
		gravity = vec3(0, 0, -4.f);

		iterations = 3;

		biasSlop = 0.005f;
		biasFactor = 0.2f;

		elasticity = 0.4f;
		friction = 0.55f;
	}

	void setEntityComponentSystemPtr(ECS::ECSManager* _ecs) {
		ecs = _ecs;
	}

	void update(float deltaTime);
	void tickPhysics();

	virtual void addCollisionECS(CollisionECS collision) override {
		if (collisionCount >= maxCollisions) return;
		collisionsECS[collisionCount++] = collision;
	}
	virtual void clearCollisions() override { collisionCount = 0; }
};