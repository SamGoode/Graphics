#pragma once

#include <glm/glm.hpp>

using glm::vec3;


struct PhysicsSolver {
private:
	struct IPhysicsEngine* physEngInterface;

public:
	PhysicsSolver(IPhysicsEngine* interface) : physEngInterface(interface) {}

	void solvePosition(struct Collision& collision);
	void solveImpulse(Collision& collision);
	void solveImpulseSingleBody(Collision& collision);

private:
	static float calcEffectiveMass(class PhysicsObject* object, vec3 radNorm);
	static float calcEffectiveMass(Collision& collision);
};


//class Solver {
//    //void SolveImpulsePair(Collision& collision1, Collision& collision2);
//
//    //void SolveFriction(Collision& collision);
//    //void SolveFrictionPair(Collision& collision1, Collision& collision2);
//
//    //void SolveJointPosition(struct Joint& joint);
//    //void SolveJointVelocity(Joint& joint);
//
//    //void SolveMouseJoint(struct MouseJoint& mouseJoint, float DeltaTime);
//
//    //void ApplyRestitution(Collision& collision);
//};



