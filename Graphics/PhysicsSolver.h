#pragma once

#include <glm/glm.hpp>

using glm::vec3;


struct PhysicsSolver {
private:
	struct IPhysicsEngine* physicsEngine;

public:
	PhysicsSolver(IPhysicsEngine* interface) : physicsEngine(interface) {}

	void solvePosition(struct CollisionECS& collision);
	void solveImpulse(CollisionECS& collision);
	void solveFriction(CollisionECS& collision);
	void applyRestitution(CollisionECS& collision);

private:
	//static float calcLambda(CollisionECS& collision);
	//static float calcLambda(CollisionECS& collision, vec3 direction);

	//static float calcRelativeVel(CollisionECS& collision);
	//static float calcRelativeVel(const struct PhysicsComponent& physicsCompA, vec3 radA, const PhysicsComponent& physicsCompB, vec3 radB, vec3 direction);
	static float calcRelativeVel(const struct PhysicsComponent& physicsComponent, vec3 rad, vec3 direction);

	//static float calcEffectiveMass(CollisionECS& collision);
	//static float calcEffectiveMass(const PhysicsComponent& physicsA, const TransformComponent& transformA, const PhysicsComponent& physicsB, const TransformComponent& transformB, vec3 direction);
	static float calcEffectiveMass(const PhysicsComponent& physics, const struct TransformComponent& transform, vec3 radNorm);
};


//class Solver {
//    //void SolveJointPosition(struct Joint& joint);
//    //void SolveJointVelocity(Joint& joint);
//
//    //void SolveMouseJoint(struct MouseJoint& mouseJoint, float DeltaTime);
//};



