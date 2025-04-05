#include "PhysicsEngine.h"

#include "GameEngine.h"
#include "glmAddon.h"

#include "PhysicsSystem.h"
#include "CollisionSystem.h"



void PhysicsEngine::update(float deltaTime) {
	accumulatedTime += deltaTime;

	for (int step = 0; step < maxTicksPerUpdate && accumulatedTime > fixedTimeStep; step++) {
		tickPhysics();
		accumulatedTime -= fixedTimeStep;
	}
}


void PhysicsEngine::tickPhysics() {
	PhysicsSystem* physicsSystem = ecs->getSystem<PhysicsSystem>();
	CollisionSystem* collisionSystem = ecs->getSystem<CollisionSystem>();

	// Kinematic updates and gravity
	physicsSystem->kinematicInitialUpdate(ecs, fixedTimeStep);
	physicsSystem->applyGravity(ecs, gravity);

	// Collision detection
	collisionSystem->detectCollisions(ecs, this);

	// Depenetration with pseudo position impulses
	for (int i = 0; i < iterations; i++) {
		for (int n = 0; n < collisionCount; n++) {
			solver.solvePosition(collisionsECS[n]);
		}
	}

	// Solves collisions at velocity level
	for (int i = 0; i < iterations; i++) {
		// Applies friction
		for (int n = 0; n < collisionCount; n++) {
			solver.solveFriction(collisionsECS[n]);
		}

		// Applies normal impulse
		for (int n = 0; n < collisionCount; n++) {
			solver.solveImpulse(collisionsECS[n]);
		}
	}

	// Applies restitution
	for (int i = 0; i < collisionCount; i++) {
		solver.applyRestitution(collisionsECS[i]);
	}

	// Clear collision data
	clearCollisions();

	// Velocity-verlet related
	physicsSystem->kinematicFinalUpdate(ecs, fixedTimeStep);
}









