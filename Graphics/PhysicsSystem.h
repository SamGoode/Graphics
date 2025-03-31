#pragma once

#include "ECS.h"
#include "ECSComponents.h"


class PhysicsSystem : public ECS::System {
public:
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
			transformComp.position += physicsComp.vel * deltaTime;
		}
	}
};