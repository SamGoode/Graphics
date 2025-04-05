#pragma once

#include "ECS.h"
#include "glm/ext.hpp"


struct IPhysicsEngine {
	ECS::ECSManager* ecs;

	glm::vec3 gravity;

	int iterations;

	float biasSlop;
	float biasFactor;

	float elasticity;
	float friction;

	virtual void addCollisionECS(struct CollisionECS collision) = 0;
	virtual void clearCollisions() = 0;
};