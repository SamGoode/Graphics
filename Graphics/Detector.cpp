#include "Detector.h"

#include <algorithm>

#include "IPhysicsEngine.h"
#include "PhysicsBody.h"
#include "Collision.h"


void getGlobalBoxVerts(vec3 verts[8], PhysicsObject* box) {
	vec3 extents = static_cast<Box*>(box->shape)->extents;

	for (int i = 0; i < 8; i++) {
		vec3 permutation = vec3((i >> 2) & 1, (i >> 1) & 1, i & 1);
		vec3 vertex = extents * (permutation * 2.f - vec3(1));

		verts[i] = box->pos + box->rot * vertex;
	}
}

void getMinMaxProjection(vec3 axis, vec3 points[8], float& min, float& max) {
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


void Detector::checkCollision(PhysicsObject* body, Plane plane) {
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

			physEngInterface->addCollision(collision);
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

			physEngInterface->addCollision(collision);
		}
	}
}

void Detector::checkCollision(PhysicsObject* A, PhysicsObject* B) {
	if (A->getID() >= B->getID()) {
		PhysicsObject* temp = A;
		A = B;
		B = temp;
	}

	int type = A->getID() + B->getID();
	(*this.*f[type])(A, B);
}

void Detector::checkCollision00(PhysicsObject* boxA, PhysicsObject* boxB) {
	vec3 vertsA[8];
	vec3 vertsB[8];
	getGlobalBoxVerts(vertsA, boxA);
	getGlobalBoxVerts(vertsB, boxB);

	vec3 axisNorms[3] = {
		vec3(1, 0, 0),
		vec3(0, 1, 0),
		vec3(0, 0, 1)
	};

	vec3 axes[15];

	for (int i = 0; i < 3; i++) {
		axes[i] = boxA->rot * axisNorms[i]; // A face normals
		axes[i + 3] = boxB->rot * axisNorms[i]; // B face normals
	}

	// co-planar edge plane normals
	for (int i = 0; i < 3; i++) {
		for (int n = 0; n < 3; n++) {
			axes[(i * 3 + n) + 6] = normalize(cross(axes[i], axes[n + 3]));
		}
	}

	vec3 worldNorm = vec3(0);
	float projectedPointA = 0;
	float projectedPointB = 0;

	float minOverlap = FLT_MAX;
	int minIndex = -1;

	// axes[0-15]
	// 0-2 A face norms
	// 3-5 B face norms
	// 6-14 edge plane norms

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

		if (maxA - minB < maxB - minA) {
			// axis direction A to B
			worldNorm = axis;
			projectedPointA = maxA;
			projectedPointB = minB;
		}
		else {
			// axis direction B to A
			worldNorm = -axis; // flips to A to B

			// remains unflipped
			projectedPointA = minA;
			projectedPointB = maxB;
		}
	}

	vec3 pointA;
	vec3 pointB;
	float depth;

	if (minIndex < 3) {
		for (int i = 0; i < 8; i++) {
			if (abs(projectedPointB - dot(vertsB[i], axes[minIndex])) < 0.001f) {
				// Found matching projection
				pointB = vertsB[i];
				pointA = pointB + worldNorm * minOverlap;
				depth = minOverlap;
				break;
			}
		}
	}
	else if (minIndex < 6) {
		for (int i = 0; i < 8; i++) {
			if (abs(projectedPointA - dot(vertsA[i], axes[minIndex])) < 0.001f) {
				// Found matching projection
				pointA = vertsA[i];
				pointB = pointA - worldNorm * minOverlap;
				depth = minOverlap;
				break;
			}
		}
	}
	else {
		vec3 vertA;
		vec3 vertB;
		for (int i = 0; i < 8; i++) {
			if (abs(projectedPointA - dot(vertsA[i], axes[minIndex])) < 0.001f) {
				// Found matching projection
				vertA = vertsA[i];
				break;
			}
		}
		for (int i = 0; i < 8; i++) {
			if (abs(projectedPointB - dot(vertsB[i], axes[minIndex])) < 0.001f) {
				// Found matching projection
				vertB = vertsB[i];
				break;
			}
		}

		vec3 edgeA = axes[(minIndex - 6) / 3];
		vec3 edgeB = axes[(minIndex - 6) % 3];

		pointA = vertA + edgeA * (dot(edgeA, vertB - vertA));
		pointB = vertB + edgeB * (dot(edgeB, vertA - vertB));
		depth = length(pointA - pointB);
	}

	Collision collision = {
		.bodyA = boxA,
		.bodyB = boxB,
		.worldNormal = worldNorm,
		.pointA = pointA,
		.pointB = pointB,
		.depth = depth
	};


	physEngInterface->addCollision(collision);
}


void Detector::checkCollision01(PhysicsObject* box, PhysicsObject* sphere) {
	vec3 toSphere = sphere->pos - box->pos;
	vec3 localToSphere = toSphere * box->rot;
	float radius = static_cast<Sphere*>(sphere->shape)->radius;
	vec3 extents = static_cast<Box*>(box->shape)->extents;
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

	vec3 worldNorm = box->rot * (norm * undoAbs);

	vec3 pointA = box->pos + box->rot * (extents * undoAbs);
	vec3 pointB = sphere->pos - worldNorm * radius;

	Collision collision = {
		.bodyA = box,
		.bodyB = sphere,
		.worldNormal = worldNorm,
		.pointA = pointA,
		.pointB = pointB,
		.depth = depth
	};

	physEngInterface->addCollision(collision);
}

void Detector::checkCollision11(PhysicsObject* sphereA, PhysicsObject* sphereB) {
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

	physEngInterface->addCollision(collision);
}


//bool Detector::IsWithinBody(PhysicsBody* body, Vector2 pos) {
//    switch (body->GetID()) {
//    case 0:
//    {
//        RigidRect* rect = dynamic_cast<RigidRect*>(body);
//
//        Vector2 toPos = Vector2Rotate(pos - rect->pos, -rect->rot);
//        Vector2 absToPos = { abs(toPos.x), abs(toPos.y) };
//
//        return (absToPos.x < rect->width / 2 && absToPos.y < rect->height / 2);
//    }
//        break;
//
//    case 1:
//        RigidCircle* circle = dynamic_cast<RigidCircle*>(body);
//
//        return Vector2DistanceSqr(circle->pos, pos) < circle->radius * circle->radius;
//        break;
//    }
//
//    return false;
//}
