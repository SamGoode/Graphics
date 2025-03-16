#include "PhysicsEngine.h"

#include "GameEngine.h"
#include <iostream>


void PhysicsEngine::update(float deltaTime) {
	Plane ground = parent->ground;
	int bodyCount = registry<PhysicsBody>::count;
	PhysicsBody** bodies = registry<PhysicsBody>::entries;

	// Kinematic updates and gravity
	for (int i = 0; i < bodyCount; i++) {
		bodies[i]->update(deltaTime);

		vec3 gravity = vec3(0, 0, -1.5f);
		bodies[i]->acc += gravity;
	}

	// Ground collisions
	for (int i = 0; i < bodyCount; i++) {
		checkCollision(bodies[i], ground);
	}

	// Calculates collision impulses
	for (int i = 0; i < iterations; i++) {
		for (int n = 0; n < collisionCount; n++) {
			solveImpulse(collisions[n]);
		}
	}
	clearCollisions();

	// Velocity-verlet related
	for (int i = 0; i < bodyCount; i++) {
		bodies[i]->finaliseUpdate(deltaTime);
	}
}


void PhysicsEngine::solveImpulse(Collision& collision) {
	if (!collision.bodyB) {
		solveImpulseSingleBody(collision);
		return;
	}

	PhysicsBody* A = collision.bodyA;
	PhysicsBody* B = collision.bodyB;

	vec3 Va = A->vel;
	vec3 Wa = A->angVel;
	vec3 Vb = B->vel;
	vec3 Wb = B->angVel;

	float iMa = A->invMass;
	mat3 iIa = A->invInertia;
	float iMb = B->invMass;
	mat3 iIb = B->invInertia;

	vec3 rA = collision.pointA - A->pos;
	vec3 rB = collision.pointB - B->pos;
	vec3 norm = collision.worldNormal;

	float JV = dot(-norm, Va) + dot(cross(-rA, norm), Wa) + dot(norm, Vb) + dot(cross(rB, norm), Wb);
	float effMass = iMa + dot(cross(-rA, norm), A->rot * (iIa * cross(-rA, norm) * A->rot)) + iMb + dot(cross(rB, norm), B->rot * (iIb * cross(rB, norm) * B->rot));

	float lambda = -JV / effMass;
	float newSum = std::max(collision.lambdaSum + lambda, 0.f);

	lambda = newSum - collision.lambdaSum;
	collision.lambdaSum += lambda;

	A->applyImpulse(norm * -lambda, collision.pointA);
	B->applyImpulse(norm * lambda, collision.pointB);
}

void PhysicsEngine::solveImpulseSingleBody(Collision& collision) {
	PhysicsBody* A = collision.bodyA;
	
	vec3 Va = A->vel;
	vec3 Wa = A->angVel;

	float iMa = A->invMass;
	mat3 iIa = A->invInertia;

	vec3 rA = collision.pointA - A->pos;

	vec3 norm = collision.worldNormal;

	float JV = dot(-norm, Va) + dot(cross(-rA, norm), Wa);
	float effMass = iMa + dot(cross(-rA, norm), A->rot * (iIa * (cross(-rA, norm) * A->rot)));

	float lambda = -JV / effMass;
	float newSum = std::max(collision.lambdaSum + lambda, 0.f);

	lambda = newSum - collision.lambdaSum;
	collision.lambdaSum += lambda;

	A->applyImpulse(norm * -lambda, collision.pointA);
}

void PhysicsEngine::checkCollision(PhysicsBody* body, Plane plane) {
	switch (body->getID()) {
	case 0:
		vec3 extents = dynamic_cast<Box*>(body->shape)->extents;

		for (int n = 0; n < 8; n++) {
			vec3 vertOffset = vec3((n >> 2) & 1, (n >> 1) & 1, n & 1);
			vertOffset = extents * (vertOffset * 2.f - vec3(1));

			vec3 vertex = body->pos + (body->rot * vertOffset);
			if (!plane.isPointUnderPlane(vertex)) { continue; }

			Collision collision = {
				.bodyA = body,
				.worldNormal = -plane.normal,
				.pointA = vertex
			};

			addCollision(collision);
		}
		break;

	case 1:
		float radius = dynamic_cast<Sphere*>(body->shape)->radius;

		if (dot(body->pos, plane.normal) - radius < -plane.distanceFromOrigin) {
			Collision collision = {
				.bodyA = body,
				.worldNormal = -plane.normal,
				.pointA = body->pos - (plane.normal * radius)
			};

			addCollision(collision);
		}
	}
}
