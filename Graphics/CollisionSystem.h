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
			ECS::uint entityA = entities[i];
			CollisionComponent& collisionCompA = manager->getComponent<CollisionComponent>(entityA);
			TransformComponent& transformCompA = manager->getComponent<TransformComponent>(entityA);

			for (int n = i + 1; n < entityCount; n++) {
				ECS::uint entityB = entities[n];
				CollisionComponent& collisionCompB = manager->getComponent<CollisionComponent>(entityB);
				TransformComponent& transformCompB = manager->getComponent<TransformComponent>(entityB);

				//unsigned int collisionType = collisionCompA.geometry + collisionCompB.geometry; // fix this so it works with more than three types

				// triangular matrix
				// 0
				// 1 2
				// 3 4 5

				// sum of all numbers between 100 is (1 + 100) + (2 + 99) + (3 + 98)... = 101 * 50 = 5050
				// n(n + 1) / 2

				if (collisionCompA.geometry > collisionCompB.geometry) {
					unsigned int n = collisionCompA.geometry;
					unsigned int collisionType = collisionCompB.geometry + (n * (n + 1)) / 2;

					// switcheroo
					(*this.*collisionFunctions[collisionType])(physicsEngine, entityB, transformCompB, entityA, transformCompA);
				}
				else {
					unsigned int n = collisionCompB.geometry;
					unsigned int collisionType = collisionCompA.geometry + (n * (n + 1)) / 2;

					(*this.*collisionFunctions[collisionType])(physicsEngine, entityA, transformCompA, entityB, transformCompB);
				}
			}
		}
	}

private:
	typedef void (CollisionSystem::* func)(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent transformA, ECS::uint entityB, TransformComponent transformB);
	func collisionFunctions[6] = {
		&CollisionSystem::checkCollisionBoxBox,
		&CollisionSystem::checkCollisionBoxSphere,
		&CollisionSystem::checkCollisionSphereSphere,
		&CollisionSystem::checkCollisionBoxPlane,
		&CollisionSystem::checkCollisionSpherePlane,
		&CollisionSystem::checkCollisionPlanePlane
	};

	void checkCollisionBoxBox(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent boxA, ECS::uint entityB, TransformComponent boxB);
	void checkCollisionBoxSphere(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent box, ECS::uint entityB, TransformComponent sphere);
	void checkCollisionSphereSphere(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent sphereA, ECS::uint entityB, TransformComponent sphereB);
	void checkCollisionBoxPlane(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent box, ECS::uint entityB, TransformComponent plane);
	void checkCollisionSpherePlane(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent sphere, ECS::uint entityB, TransformComponent plane);
	void checkCollisionPlanePlane(IPhysicsEngine* physicsEngine, ECS::uint entityA, TransformComponent planeA, ECS::uint entityB, TransformComponent planeB);

	static void getGlobalBoxVerts(vec3 verts[8], TransformComponent box);
	static void getMinMaxProjection(vec3 axis, vec3 points[8], float& min, float& max);
};

