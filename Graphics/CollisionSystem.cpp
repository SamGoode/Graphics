#include "CollisionSystem.h"


void CollisionSystem::checkCollisionBoxBox(IPhysicsEngine* physicsEngine, ECS::uint entityA, const TransformComponent& boxA, ECS::uint entityB, const TransformComponent& boxB) {
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
			if (abs(dot(axes[i], axes[n + 3])) > 0.9999f) {
				axes[(i * 3 + n) + 6] = vec3(0);
				continue;
			}

			axes[(i * 3 + n) + 6] = normalize(cross(axes[i], axes[n + 3]));
		}
	}

	int minIndex = -1;
	float minOverlap = FLT_MAX;
	vec3 worldNorm = vec3(0);

	for (int i = 0; i < 15; i++) {
		vec3 axis = axes[i];

		if (axis == vec3(0)) {
			continue;
		}

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
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		// I have a hunch something here occasionally gives inaccurate values during certain edge cases.

		physicsEngine->addCollisionECS(collision);

		//std::cout << "t1: " << t1 << std::endl << "t2: " << t2 << std::endl;
		//std::cout << minOverlap << std::endl;
		//std::cout << length(collision.pointA - collision.pointB) << std::endl;
		//std::cout << "length: " << glm::distance(edgeBaseB, edgeBaseB + edgeNormB * extentsB[edgeNormIndexB]) << std::endl;
	}
}

void CollisionSystem::checkCollisionBoxSphere(IPhysicsEngine* physicsEngine, ECS::uint entityA, const TransformComponent& box, ECS::uint entityB, const TransformComponent& sphere) {
	vec3 extents = box.scale * 0.5f;
	float radius = (sphere.scale.x + sphere.scale.y + sphere.scale.z) / 3.f; // should be the same anyway

	vec3 toSphere = sphere.position - box.position;
	vec3 localToSphere = toSphere * box.rotation;

	vec3 absOffset = abs(localToSphere);
	vec3 vertexOffset = absOffset - extents;


	if (length(max(vertexOffset, vec3(0))) > radius) {
		// entire sphere outside box
		return;
	}

	vec3 norm = vec3(0);
	float depth = 0;

	int maxIndex = 0;
	float maxAxisOffset = vertexOffset[0];
	for (int i = 1; i < 3; i++) {
		if (vertexOffset[i] > maxAxisOffset) {
			maxAxisOffset = vertexOffset[i];
			maxIndex = i;
		}
	}

	if (maxAxisOffset < 0) {
		// sphere centre inside box
		norm[maxIndex] = 1;
		depth = -maxAxisOffset + radius;
	}
	else {
		// sphere centre outside box
		norm = glm::normalize(max(vertexOffset, vec3(0)));
		depth = -length(max(vertexOffset, vec3(0))) + radius;
	}

	vec3 undoAbs; //= localToSphere / absOffset;
	undoAbs.x = localToSphere.x < 0 ? -1 : 1;
	undoAbs.y = localToSphere.y < 0 ? -1 : 1;
	undoAbs.z = localToSphere.z < 0 ? -1 : 1;

	vec3 worldNorm = box.rotation * (norm * undoAbs);

	vec3 pointA = box.position + box.rotation * (extents * undoAbs);
	vec3 pointB = sphere.position - worldNorm * radius;

	CollisionECS collision = {
		.entityA = entityA,
		.entityB = entityB,
		.worldNormal = worldNorm,
		.pointA = pointA,
		.pointB = pointB,
		.depth = depth
	};

	physicsEngine->addCollisionECS(collision);
}

void CollisionSystem::checkCollisionSphereSphere(IPhysicsEngine* physicsEngine, ECS::uint entityA, const TransformComponent& sphereA, ECS::uint entityB, const TransformComponent& sphereB) {
	float radiusA = (sphereA.scale.x + sphereA.scale.y + sphereA.scale.z) / 3.f;
	float radiusB = (sphereB.scale.x + sphereB.scale.y + sphereB.scale.z) / 3.f;
	float radii = radiusA + radiusB;

	vec3 AtoB = sphereB.position - sphereA.position;
	float sqrDist = dot(AtoB, AtoB);

	if (sqrDist > radii * radii) {
		return;
	}

	float dist = sqrt(sqrDist);
	vec3 norm = AtoB * (1 / dist);

	CollisionECS collision = {
		.entityA = entityA,
		.entityB = entityB,
		.worldNormal = norm,
		.pointA = sphereA.position + norm * radiusA,
		.pointB = sphereB.position - norm * radiusB,
		.depth = radii - dist
	};

	physicsEngine->addCollisionECS(collision);
}

void CollisionSystem::checkCollisionBoxPlane(IPhysicsEngine* physicsEngine, ECS::uint entityA, const TransformComponent& box, ECS::uint entityB, const TransformComponent& plane) {
	vec3 extents = box.scale * 0.5f;

	// assume planes face up by default
	vec3 planeNormal = plane.rotation * vec3(0, 0, 1);

	// for now planes will be infinite regardless of their scale

	for (int n = 0; n < 8; n++) {
		vec3 vertOffset = vec3((n >> 2) & 1, (n >> 1) & 1, n & 1);
		vertOffset = extents * (vertOffset * 2.f - vec3(1));

		vec3 vertex = box.position + (box.rotation * vertOffset);

		float separation = dot(vertex - plane.position, planeNormal);
		if (separation > 0.f) { continue; }

		CollisionECS collision = {
			.entityA = entityA,
			.entityB = entityB,
			.worldNormal = -planeNormal,
			.pointA = vertex,
			.depth = -separation
		};

		physicsEngine->addCollisionECS(collision);
	}
}

void CollisionSystem::checkCollisionSpherePlane(IPhysicsEngine* physicsEngine, ECS::uint entityA, const TransformComponent& sphere, ECS::uint entityB, const TransformComponent& plane) {
	float radius = (sphere.scale.x + sphere.scale.y + sphere.scale.z) / 3.f; // should be the same anyway

	// assume planes face up by default
	vec3 planeNormal = plane.rotation * vec3(0, 0, 1);

	float separation = dot(sphere.position - plane.position, planeNormal) - radius;
	if (separation < 0.f) {
		CollisionECS collision = {
			.entityA = entityA,
			.entityB = entityB,
			.worldNormal = -planeNormal,
			.pointA = sphere.position - (planeNormal * radius),
			.depth = -separation
		};

		physicsEngine->addCollisionECS(collision);
	}
}

void CollisionSystem::checkCollisionPlanePlane(IPhysicsEngine* physicsEngine, ECS::uint entityA, const TransformComponent& planeA, ECS::uint entityB, const TransformComponent& planeB) {
	// Do nothing

}



void CollisionSystem::getGlobalBoxVerts(vec3 verts[8], const TransformComponent& box) {
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