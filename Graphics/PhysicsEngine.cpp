#include "PhysicsEngine.h"

#include "GameEngine.h"
#include "glmAddon.h"

#include "PhysicsSystem.h"


void PhysicsEngine::update(float deltaTime) {
	int bodyCount = Registry<PhysicsObject>::count;
	PhysicsObject** bodies = Registry<PhysicsObject>::entries;

	PhysicsSystem* physicsSystem = ecs->getSystem<PhysicsSystem>();
	
	

	// Kinematic updates and gravity
	physicsSystem->kinematicInitialUpdate(ecs, deltaTime);
	physicsSystem->applyGravity(ecs, gravity);
	//for (int i = 0; i < bodyCount; i++) {
	//	bodies[i]->update(deltaTime);

	//	vec3 gravity = vec3(0, 0, -1.5f);
	//	bodies[i]->acc += gravity;
	//}

	// Ground collisions
	for (int i = 0; i < bodyCount; i++) {
		detector.checkCollision(bodies[i], ground);

		for (int n = i + 1; n < bodyCount; n++) {
			detector.checkCollision(bodies[i], bodies[n]);
		}
	}

	// Depenetration with pseudo impulses
	for (int i = 0; i < iterations; i++) {
		for (int n = 0; n < collisionCount; n++) {
			solver.solvePosition(collisions[n]);
		}
	}

	// Solves collision impulses
	for (int i = 0; i < iterations; i++) {
		for (int n = 0; n < collisionCount; n++) {
			solver.solveFriction(collisions[n]);
		}

		for (int n = 0; n < collisionCount; n++) {
			solver.solveImpulse(collisions[n]);
		}
	}

	for (int i = 0; i < collisionCount; i++) {
		solver.applyRestitution(collisions[i]);
	}
	clearCollisions();

	// Velocity-verlet related
	physicsSystem->kinematicFinalUpdate(ecs, deltaTime);
	//for (int i = 0; i < bodyCount; i++) {
	//	bodies[i]->finaliseUpdate(deltaTime);
	//}
}









