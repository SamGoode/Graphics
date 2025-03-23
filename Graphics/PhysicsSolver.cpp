#include "PhysicsSolver.h"

#include "IPhysicsEngine.h"
#include "Collision.h"
#include "PhysicsBody.h"

#include <algorithm>
#include "glmAddon.h"



// Depenetration through Pseudo Impulse
void PhysicsSolver::solvePosition(Collision& collision) {
    float biasSlop = physEngInterface->biasSlop;
    float biasFactor = physEngInterface->biasFactor;

    float effMass = calcEffectiveMass(collision);
    float lambda = std::max(collision.depth - biasSlop, 0.f) * biasFactor / effMass;

    vec3 norm = collision.worldNormal;
    vec3 impulse = norm * lambda;

    collision.bodyA->applyPositionImpulse(-impulse, collision.pointA);
    if (collision.bodyB)
        collision.bodyB->applyPositionImpulse(impulse, collision.pointB);
}

// Normal Impulse
void PhysicsSolver::solveImpulse(Collision& collision) {
    float lambda = calcLambda(collision);
    float newSum = std::max(collision.lambdaSum + lambda, 0.f);
    lambda = newSum - collision.lambdaSum;
    collision.lambdaSum += lambda;

    vec3 norm = collision.worldNormal;
    vec3 impulse = norm * lambda;

    collision.bodyA->applyImpulse(-impulse, collision.pointA);
    if (collision.bodyB)
        collision.bodyB->applyImpulse(impulse, collision.pointB);
}


// Friction
void PhysicsSolver::solveFriction(Collision& collision) {
    vec3 norm = collision.worldNormal;
    vec3 tangent1 = { -norm.y, norm.x, 0.f };
    if (length(tangent1) == 0.f) {
        tangent1 = { 0.f, -norm.z, norm.y };
    }
    tangent1 = normalize(tangent1);
    vec3 tangent2 = cross(norm, tangent1);

    float maxFriction = collision.lambdaSum * physEngInterface->friction;

    float tangentLambda1 = calcLambda(collision, tangent1);
    float newSum1 = std::clamp(collision.tangentLambdaSum1 + tangentLambda1, -maxFriction, maxFriction);
    tangentLambda1 = newSum1 - collision.tangentLambdaSum1;
    collision.tangentLambdaSum1 += tangentLambda1;

    float tangentLambda2 = calcLambda(collision, tangent2);
    float newSum2 = std::clamp(collision.tangentLambdaSum2 + tangentLambda2, -maxFriction, maxFriction);
    tangentLambda2 = newSum2 - collision.tangentLambdaSum2;
    collision.tangentLambdaSum2 += tangentLambda2;

    vec3 frictionImpulse = tangent1 * tangentLambda1 + tangent2 * tangentLambda2;

    collision.bodyA->applyImpulse(-frictionImpulse, collision.pointA);
    if (collision.bodyB)
        collision.bodyB->applyImpulse(frictionImpulse, collision.pointB);
}

void PhysicsSolver::applyRestitution(Collision& collision) {
    vec3 norm = collision.worldNormal;
    float lambda = collision.lambdaSum;

    vec3 impulse = norm * lambda * physEngInterface->elasticity;

    collision.bodyA->applyImpulse(-impulse, collision.pointA);
    if (collision.bodyB)
        collision.bodyB->applyImpulse(impulse, collision.pointB);
}


// Shorthand
float PhysicsSolver::calcLambda(Collision& collision) {
    float JV = calcRelativeVel(collision);
    float effMass = calcEffectiveMass(collision);
    return -JV / effMass;
}

float PhysicsSolver::calcLambda(Collision& collision, vec3 direction) {
    float JV = calcRelativeVel(collision, direction);
    float effMass = calcEffectiveMass(collision, direction);
    return -JV / effMass;
}


// Calculates Jacobian * Velocity
float PhysicsSolver::calcRelativeVel(Collision& collision) {
    vec3 norm = collision.worldNormal;
    return calcRelativeVel(collision, norm);
}

float PhysicsSolver::calcRelativeVel(Collision& collision, vec3 direction) {
    PhysicsObject* A = collision.bodyA;
    PhysicsObject* B = collision.bodyB;

    vec3 rA = collision.pointA - A->pos;
    float JVA = calcRelativeVel(A, rA, -direction);

    vec3 rB = B ? collision.pointB - B->pos : vec3(0);
    float JVB = B ? calcRelativeVel(B, rB, direction) : 0.f;

    return JVA + JVB;
}

float PhysicsSolver::calcRelativeVel(PhysicsObject* object, vec3 rad, vec3 norm) {
    vec3 Va = object->vel;
    vec3 Wa = object->angVel;

    float JV = dot(norm, Va) + dot(cross(rad, norm), Wa);
    return JV;
}


// Calculates effective mass of contact constraint
float PhysicsSolver::calcEffectiveMass(Collision& collision) {
    vec3 norm = collision.worldNormal;
    return calcEffectiveMass(collision, norm);
}

float PhysicsSolver::calcEffectiveMass(Collision& collision, vec3 direction) {
    PhysicsObject* A = collision.bodyA;
    PhysicsObject* B = collision.bodyB;

    vec3 rA = collision.pointA - A->pos;
    float effMassA = calcEffectiveMass(A, cross(-rA, direction));

    vec3 rB = B ? collision.pointB - B->pos : vec3(0);
    float effMassB = B ? calcEffectiveMass(B, cross(rB, direction)) : 0.f;

    return effMassA + effMassB;
}

float PhysicsSolver::calcEffectiveMass(PhysicsObject* object, vec3 radNorm) {
    float iMa = object->invMass;
    mat3 iIa = object->invInertia;

    float effMass = iMa + dot(radNorm, object->rot * (iIa * radNorm * object->rot));
    return effMass;
}



// Joints
//void Solver::SolveJointPosition(Joint& joint) {
//    RigidBody* A = joint.bodyA;
//    RigidBody* B = joint.bodyB;
//
//    float invMassA = A->invMass;
//    float invMOIA = A->invMOI;
//
//    Vector2 radA = Vector2Rotate(joint.localA, A->rot);
//    Vector2 radPerpA = { -radA.y, radA.x };
//
//    float invMassB = B->invMass;
//    float invMOIB = B->invMOI;
//
//    Vector2 radB = Vector2Rotate(joint.localB, B->rot);
//    Vector2 radPerpB = { -radB.y, radB.x };
//
//    Vector2 pointA = A->pos + radA;
//    Vector2 pointB = B->pos + radB;
//
//    // Have to invert it later.
//    Vector2 AtoB = pointA - pointB;
//
//    float a = invMassA + invMassB + (radA.y * radA.y * invMOIA) + (radB.y * radB.y * invMOIB);
//    float b = -(radA.x * radA.y * invMOIA) - (radB.x * radB.y * invMOIB);
//    float c = -(radA.x * radA.y * invMOIA) - (radB.x * radB.y * invMOIB);
//    float d = invMassA + invMassB + (radA.x * radA.x * invMOIA) + (radB.x * radB.x * invMOIB);
//
//    float determinant = a * d - b * c;
//
//    a = a / determinant;
//    b = b / determinant;
//    c = c / determinant;
//    d = d / determinant;
//
//    Vector2 impulse;
//    impulse.x = (d * AtoB.x - b * AtoB.y) * biasFactor;
//    impulse.y = (-c * AtoB.x + a * AtoB.y) * biasFactor;
//
//    A->pos += impulse * -invMassA;
//    A->rot += Vector2DotProduct(impulse * -1, radPerpA) * invMOIA;
//
//    B->pos += (impulse * invMassB);
//    B->rot += Vector2DotProduct(impulse, radPerpB) * invMOIB;
//}
//
//void Solver::SolveJointVelocity(Joint& joint) {
//    RigidBody* A = joint.bodyA;
//    RigidBody* B = joint.bodyB;
//
//    Vector2 velA = A->vel;
//    float angVelA = A->angVel;
//
//    float invMassA = A->invMass;
//    float invMOIA = A->invMOI;
//
//    Vector2 radA = Vector2Rotate(joint.localA, A->rot);
//    Vector2 radPerpA = { -radA.y, radA.x };
//
//    Vector2 velB = B->vel;
//    float angVelB = B->angVel;
//
//    float invMassB = B->invMass;
//    float invMOIB = B->invMOI;
//
//    Vector2 radB = Vector2Rotate(joint.localB, B->rot);
//    Vector2 radPerpB = { -radB.y, radB.x };
//
//    Vector2 relVel = (velA * -1) + (radPerpA * -angVelA) + velB + (radPerpB * angVelB);
//    relVel = relVel * -1;
//
//    float a = invMassA + invMassB + (radA.y * radA.y * invMOIA) + (radB.y * radB.y * invMOIB);
//    float b = -(radA.x * radA.y * invMOIA) - (radB.x * radB.y * invMOIB);
//    float c = -(radA.x * radA.y * invMOIA) - (radB.x * radB.y * invMOIB);
//    float d = invMassA + invMassB + (radA.x * radA.x * invMOIA) + (radB.x * radB.x * invMOIB);
//
//    float determinant = a * d - b * c;
//
//    a = a / determinant;
//    b = b / determinant;
//    c = c / determinant;
//    d = d / determinant;
//
//    Vector2 impulse;
//    impulse.x = d * relVel.x - b * relVel.y;
//    impulse.y = -c * relVel.x + a * relVel.y;
//
//    A->ApplyImpulse(impulse * -1, A->pos + radA);
//    B->ApplyImpulse(impulse, B->pos + radB);
//}
//
//void Solver::SolveMouseJoint(MouseJoint& mouseJoint, float DeltaTime) {
//    RigidBody* A = mouseJoint.bodyA;
//
//    Vector2 velA = A->vel;
//    float angVelA = A->angVel;
//
//    float invMassA = A->invMass;
//    float invMOIA = A->invMOI;
//
//    Vector2 radA = Vector2Rotate(mouseJoint.localA, A->rot);
//    Vector2 radPerpA = { -radA.y, radA.x };
//    Vector2 pointA = A->pos + radA;
//
//    // Inverted
//    Vector2 biasVector = (mouseJoint.mousePos - pointA) * biasFactor / DeltaTime;
//    
//
//    Vector2 relVel = (velA * -1) + (radPerpA * -angVelA);
//    relVel += biasVector;
//    relVel = relVel * -1;
//
//    float a = invMassA + (radA.y * radA.y * invMOIA);
//    float b = -(radA.x * radA.y * invMOIA);
//    float c = -(radA.x * radA.y * invMOIA);
//    float d = invMassA + (radA.x * radA.x * invMOIA);
//
//    float determinant = a * d - b * c;
//
//    a = a / determinant;
//    b = b / determinant;
//    c = c / determinant;
//    d = d / determinant;
//
//    Vector2 impulse;
//    impulse.x = d * relVel.x - b * relVel.y;
//    impulse.y = -c * relVel.x + a * relVel.y;
//
//    A->ApplyImpulse(impulse * -1, A->pos + radA);
//}