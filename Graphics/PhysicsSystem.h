#pragma once

#include "ECS.h"
#include "ECSComponents.h"


class PhysicsSystem : public ECS::System {
public:
	void generateInertiaTensors(ECS::ECSManager* manager) {
		for (int i = 0; i < entityCount; i++) {
			auto& physicsComp = manager->getComponent<PhysicsComponent>(entities[i]);
			const auto& transformComp = manager->getComponent<TransformComponent>(entities[i]);
			// for now assume all physics objects also have a collision component - FIX LATER
			const auto& collisionComp = manager->getComponent<CollisionComponent>(entities[i]);

			float mass = 1.f / physicsComp.invMass; // assume non-zero
			vec3 dimensions = transformComp.scale;

			mat3 invInertia = glm::identity<mat3>();

			switch (collisionComp.geometry) {
			case enumGeometry::BOX:
				invInertia[0][0] = 12.f / (mass * (dimensions.y * dimensions.y + dimensions.z * dimensions.z));
				invInertia[1][1] = 12.f / (mass * (dimensions.x * dimensions.x + dimensions.z * dimensions.z));
				invInertia[2][2] = 12.f / (mass * (dimensions.x * dimensions.x + dimensions.y * dimensions.y));
				break;
			case enumGeometry::SPHERE:
				float radius = (dimensions.x + dimensions.y + dimensions.z) / 3.f; // should be the same anyway
				invInertia *= (2 * mass * radius * radius) / 5.f;
				break;
			}

			physicsComp.invInertia = invInertia;
		}
	}

	void kinematicInitialUpdate(ECS::ECSManager* manager, float deltaTime) {
		for (int i = 0; i < entityCount; i++) {
			TransformComponent& transformComp = manager->getComponent<TransformComponent>(entities[i]);
			PhysicsComponent& physicsComp = manager->getComponent<PhysicsComponent>(entities[i]);

			physicsComp.vel += physicsComp.acc * deltaTime * 0.5f;
			transformComp.position += physicsComp.vel * deltaTime;
			physicsComp.acc = vec3(0);

			physicsComp.angVel += physicsComp.angAcc * deltaTime * 0.5f;

			if (length(physicsComp.angVel) < 2.f) {
				transformComp.rotation += quat(0, physicsComp.angVel * (deltaTime * 0.5f)) * transformComp.rotation; // Approximation
			}
			else {
				transformComp.rotation = deltaRotation(physicsComp.angVel, deltaTime) * transformComp.rotation;
			}

			transformComp.rotation = normalize(transformComp.rotation);
			physicsComp.angAcc = vec3(0);
		}
	}

	void applyGravity(ECS::ECSManager* manager, vec3 gravity) {
		for (int i = 0; i < entityCount; i++) {
			PhysicsComponent& physicsComp = manager->getComponent<PhysicsComponent>(entities[i]);
			physicsComp.acc += gravity;
		}
	}

	void kinematicFinalUpdate(ECS::ECSManager* manager, float deltaTime) {
		for (int i = 0; i < entityCount; i++) {
			TransformComponent& transformComp = manager->getComponent<TransformComponent>(entities[i]);
			PhysicsComponent& physicsComp = manager->getComponent<PhysicsComponent>(entities[i]);

			physicsComp.vel += physicsComp.acc * deltaTime * 0.5f;
			physicsComp.angVel += physicsComp.angAcc * deltaTime * 0.5f;
		}
	}
};