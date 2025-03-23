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
	void solveFriction(Collision& collision);
	void applyRestitution(Collision& collision);

private:
	static float calcLambda(Collision& collision);
	static float calcLambda(Collision& collision, vec3 direction);

	static float calcRelativeVel(Collision& collision);
	static float calcRelativeVel(Collision& collision, vec3 direction);
	static float calcRelativeVel(class PhysicsObject* object, vec3 rad, vec3 norm);

	static float calcEffectiveMass(Collision& collision);
	static float calcEffectiveMass(Collision& collision, vec3 direction);
	static float calcEffectiveMass(class PhysicsObject* object, vec3 radNorm);
};


//class Solver {
//    //void SolveJointPosition(struct Joint& joint);
//    //void SolveJointVelocity(Joint& joint);
//
//    //void SolveMouseJoint(struct MouseJoint& mouseJoint, float DeltaTime);
//};



