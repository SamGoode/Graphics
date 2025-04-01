#pragma once

#include "MaterialProperties.h"

#include "glmAddon.h"

struct MeshComponent {
	int meshID;
};

struct TransformComponent {
	vec3 position;
	quat rotation;
	vec3 scale;
};

struct MaterialComponent {
	MaterialProperties material;
};

struct PhysicsComponent {
	vec3 vel;
	vec3 acc;
	float invMass;

	vec3 angVel;
	vec3 angAcc;
	mat3 invInertia;
};

namespace enumGeometry {
	enum type : unsigned int {
		BOX,
		SPHERE,
		PLANE
	};
}

struct CollisionComponent {
	enumGeometry::type geometry;
};