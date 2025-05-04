#pragma once

#include "ECS.h"
#include "glm/ext.hpp"


struct IPhysicsEngine {
	ECS::ECSManager* ecs;

	glm::vec3 gravity = glm::vec3(0);

	int iterations = 1;

	float biasSlop = 0.f;
	float biasFactor = 0.f;

	float elasticity = 0.f;
	float friction = 0.f;

	virtual void addCollisionECS(struct CollisionECS collision) = 0;
	virtual void clearCollisions() = 0;
};