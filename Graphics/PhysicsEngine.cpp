#include "PhysicsEngine.h"

#include "GameEngine.h"
#include <iostream>


void PhysicsEngine::update(float deltaTime) {
	Plane ground = parent->ground;
	int bodyCount = Registry<PhysicsObject>::count;
	PhysicsObject** bodies = Registry<PhysicsObject>::entries;

	// Kinematic updates and gravity
	for (int i = 0; i < bodyCount; i++) {
		bodies[i]->update(deltaTime);

		vec3 gravity = vec3(0, 0, -1.5f);
		bodies[i]->acc += gravity;
	}

	// Ground collisions
	for (int i = 0; i < bodyCount; i++) {
		checkCollision(bodies[i], ground);

		for (int n = i + 1; n < bodyCount; n++) {
			checkCollision(bodies[i], bodies[n]);
		}
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

	PhysicsObject* A = collision.bodyA;
	PhysicsObject* B = collision.bodyB;

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
	PhysicsObject* A = collision.bodyA;
	
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

void PhysicsEngine::checkCollision(PhysicsObject* body, Plane plane) {
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


void PhysicsEngine::checkCollision(PhysicsObject* A, PhysicsObject* B) {
	if (A->getID() >= B->getID()) {
		PhysicsObject* temp = A;
		A = B;
		B = temp;
	}

	int type = A->getID() + B->getID();
	(*this.*f[type])(A, B);
}


void PhysicsEngine::checkCollision00(PhysicsObject* boxA, PhysicsObject* boxB) {

}

void PhysicsEngine::checkCollision01(PhysicsObject* box, PhysicsObject* sphere) {

}

void PhysicsEngine::checkCollision11(PhysicsObject* sphereA, PhysicsObject* sphereB) { 
	float radiusA = static_cast<Sphere*>(sphereA->shape)->radius;
	float radiusB = static_cast<Sphere*>(sphereB->shape)->radius;
	float radii = radiusA + radiusB;

	vec3 AtoB = sphereB->pos - sphereA->pos;
	float sqrDist = dot(AtoB, AtoB);

	if (sqrDist > radii * radii) {
		return;
	}

	vec3 norm = AtoB * (1 / sqrt(sqrDist));

	Collision collision = {
		.bodyA = sphereA,
		.bodyB = sphereB,
		.worldNormal = norm,
		.pointA = sphereA->pos + norm * radiusA,
		.pointB = sphereB->pos - norm * radiusB
	};

	addCollision(collision);
}
