#pragma once

struct IPhysicsEngine {
	int iterations;

	float biasSlop;
	float biasFactor;

	float elasticity;
	float friction;

	virtual void addCollision(struct Collision collision) = 0;
	virtual void addCollisionECS(struct CollisionECS collision) = 0;
	virtual void clearCollisions() = 0;
};