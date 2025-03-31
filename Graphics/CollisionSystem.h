#pragma once

#include "ECS.h"
#include "ECSComponents.h"
#include "IPhysicsEngine.h"

#include "Collision.h"

#include "glmAddon.h"

class CollisionSystem : public ECS::System {
public:
	void detectCollisions(ECS::ECSManager* manager, IPhysicsEngine* physicsEngine) {
		for (int i = 0; i < entityCount; i++) {
			CollisionComponent& collisionCompA = manager->getComponent<CollisionComponent>(entities[i]);
			TransformComponent& transformCompA = manager->getComponent<TransformComponent>(entities[i]);

			for (int n = i + 1; n < entityCount; n++) {
				CollisionComponent& collisionCompB = manager->getComponent<CollisionComponent>(entities[n]);
				TransformComponent& transformCompB = manager->getComponent<TransformComponent>(entities[n]);

				if (A->getID() > B->getID()) {
					PhysicsObject* temp = A;
					A = B;
					B = temp;
				}

				int type = A->getID() + B->getID();
				(*this.*f[type])(A, B);
			}
		}
	}

private:
	typedef void (CollisionSystem::* func)(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent boxA, ECS::uint entityB, TransformComponent boxB);
	func f[3] = {
		&CollisionSystem::checkCollisionBoxBox,
		&CollisionSystem::checkCollisionBoxSphere,
		&CollisionSystem::checkCollisionSphereSphere
	};

	void checkCollisionBoxBox(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent boxA, ECS::uint entityB, TransformComponent boxB);
	void checkCollisionBoxSphere(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent boxA, ECS::uint entityB, TransformComponent boxB);
	void checkCollisionSphereSphere(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent boxA, ECS::uint entityB, TransformComponent boxB);

	static void getGlobalBoxVerts(vec3 verts[8], TransformComponent box);
	static void getMinMaxProjection(vec3 axis, vec3 points[8], float& min, float& max);
};



void CollisionSystem::checkCollisionBoxBox(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent boxA, ECS::uint entityB, TransformComponent boxB) {
	vec3 vertsA[8];
	vec3 vertsB[8];
	getGlobalBoxVerts(vertsA, boxA);
	getGlobalBoxVerts(vertsB, boxB);

	// axes[0-15]
	// 0-2 A face norms
	// 3-5 B face norms
	// 6-14 edge plane norms

	vec3 axes[15];
	for (int i = 0; i < 3; i++) {
		vec3 axis = vec3(0);
		axis[i] = 1;

		axes[i] = boxA.rotation * axis; // A face normals
		axes[i + 3] = boxB.rotation * axis; // B face normals
	}

	// co-planar edge plane normals
	for (int i = 0; i < 3; i++) {
		for (int n = 0; n < 3; n++) {
			axes[(i * 3 + n) + 6] = normalize(cross(axes[i], axes[n + 3]));
		}
	}

	int minIndex = -1;
	float minOverlap = FLT_MAX;
	vec3 worldNorm = vec3(0);

	for (int i = 0; i < 15; i++) {
		vec3 axis = axes[i];

		float minA;
		float maxA;
		float minB;
		float maxB;

		getMinMaxProjection(axis, vertsA, minA, maxA);
		getMinMaxProjection(axis, vertsB, minB, maxB);

		if (maxA < minB || maxB < minA) {
			// found a separating axis
			return;
		}

		float overlap = glm::min(maxA - minB, maxB - minA);
		if (overlap > minOverlap) {
			continue;
		}

		minOverlap = overlap;
		minIndex = i;

		// worldNorm should always be facing A to B
		if (maxA - minB < maxB - minA) {
			// axis direction A to B
			worldNorm = axis;
		}
		else {
			// axis direction B to A
			worldNorm = -axis; // flips to A to B
		}
	}

	CollisionECS collision = {
		.entityA = entityA,
		.entityB = entityB,
		.worldNormal = worldNorm
	};

	vec3 extentsA = boxA.scale * 0.5f;
	vec3 extentsB = boxB.scale * 0.5f;

	// Vert B on Face A
	if (minIndex < 3) {
		vec3 faceA = boxA.position + worldNorm * extentsA[minIndex]; // x,y,z index should line up.

		for (int i = 0; i < 8; i++) {
			if (dot(vertsB[i] - faceA, worldNorm) < 0.f) {
				collision.pointB = vertsB[i];
				collision.depth = dot(faceA, worldNorm) - dot(collision.pointB, worldNorm);
				collision.pointA = collision.pointB + worldNorm * collision.depth;

				physicsEngine->addCollisionECS(collision);
				//break;
			}
		}
	}
	// Vert A on Face B
	else if (minIndex < 6) {
		vec3 faceB = boxB.position - worldNorm * extentsB[minIndex - 3]; // x,y,z index should line up.

		for (int i = 0; i < 8; i++) {
			if (dot(vertsA[i] - faceB, -worldNorm) < 0.f) {
				collision.pointA = vertsA[i];
				collision.depth = dot(faceB, -worldNorm) - dot(collision.pointA, -worldNorm);
				collision.pointB = collision.pointA - worldNorm * collision.depth;

				physicsEngine->addCollisionECS(collision);
				//break;
			}
		}
	}
	// Edge on Edge collision
	else {
		int edgeNormIndexA = (minIndex - 6) / 3;
		int edgeNormIndexB = (minIndex - 6) % 3;

		vec3 edgeNormA = axes[edgeNormIndexA];
		vec3 edgeNormB = axes[edgeNormIndexB + 3];

		// If one value is zero then goes through all combinations where other two are non-zero
		//vec3 midPoint = temp[0] - temp[1] - temp[2];
		//vec3 midPoint = -temp[0] + temp[1] - temp[2];
		//vec3 midPoint = -temp[0] - temp[1] + temp[2];
		//vec3 midPoint = temp[0] + temp[1] + temp[2];

		// Finding closest edge A
		vec3 temp[3] = {
			axes[0] * extentsA.x,
			axes[1] * extentsA.y,
			axes[2] * extentsA.z
		};
		temp[edgeNormIndexA] = vec3(0.f);

		vec3 closestEdgeMidpointA;
		float maxProjectionA = -FLT_MAX;
		for (int i = 0; i < 4; i++) {
			vec3 midPoint = (i % 3) == 0 ? temp[0] : -temp[0];
			midPoint += (i % 2) == 0 ? -temp[1] : temp[1];
			midPoint += (i < 2) ? -temp[2] : temp[2];

			float projection = dot(boxA.position + midPoint, worldNorm);
			if (projection > maxProjectionA) {
				maxProjectionA = projection;
				closestEdgeMidpointA = boxA.position + midPoint;
			}
		}
		vec3 edgeBaseA = closestEdgeMidpointA - edgeNormA * extentsA[edgeNormIndexA];

		// Finding closest edge B
		vec3 tempB[3] = {
			axes[3] * extentsB.x,
			axes[4] * extentsB.y,
			axes[5] * extentsB.z
		};
		tempB[edgeNormIndexB] = vec3(0.f);

		vec3 closestEdgeMidpointB;
		float maxProjectionB = -FLT_MAX;
		for (int i = 0; i < 4; i++) {
			vec3 midPoint = (i % 3) == 0 ? tempB[0] : -tempB[0];
			midPoint += (i % 2) == 0 ? -tempB[1] : tempB[1];
			midPoint += (i < 2) ? -tempB[2] : tempB[2];


			float projection = dot(boxB.position + midPoint, -worldNorm);
			if (projection > maxProjectionB) {
				maxProjectionB = projection;
				closestEdgeMidpointB = boxB.position + midPoint;
			}
		}
		vec3 edgeBaseB = closestEdgeMidpointB - edgeNormB * extentsB[edgeNormIndexB];


		// Bishmallah let my math be right
		vec3 a = edgeBaseB - edgeBaseA;
		float x = dot(edgeNormA, edgeNormB);

		float t1 = dot(a, x * edgeNormB + edgeNormA) / (1 - (x * x));
		float t2 = t1 * x - dot(a, edgeNormB);

		// clamp to edge length
		collision.pointA = edgeBaseA + edgeNormA * std::max(std::min(t1, extentsA[edgeNormIndexA] * 2.f), 0.f);
		collision.pointB = edgeBaseB + edgeNormB * std::max(std::min(t2, extentsB[edgeNormIndexB] * 2.f), 0.f);
		collision.depth = minOverlap;

		physicsEngine->addCollisionECS(collision);

		//std::cout << "t1: " << t1 << std::endl << "t2: " << t2 << std::endl;
		//std::cout << minOverlap << std::endl;
		//std::cout << length(collision.pointA - collision.pointB) << std::endl;
		//std::cout << "length: " << glm::distance(edgeBaseB, edgeBaseB + edgeNormB * extentsB[edgeNormIndexB]) << std::endl;
	}
}


void CollisionSystem::getGlobalBoxVerts(vec3 verts[8], TransformComponent box) {
	vec3 extents = box.scale * 0.5f;

	for (int i = 0; i < 8; i++) {
		vec3 permutation = vec3((i >> 2) & 1, (i >> 1) & 1, i & 1);
		vec3 vertex = extents * (permutation * 2.f - vec3(1));

		verts[i] = box.position + box.rotation * vertex;
	}
}

void CollisionSystem::getMinMaxProjection(vec3 axis, vec3 points[8], float& min, float& max) {
	float projection = dot(points[0], axis);
	max = projection;
	min = projection;

	for (int i = 1; i < 8; i++) {
		projection = dot(points[i], axis);

		if (projection > max) {
			max = projection;
		}
		if (projection < min) {
			min = projection;
		}
	}
}