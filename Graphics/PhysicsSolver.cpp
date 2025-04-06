#include "PhysicsSolver.h"

#include "IPhysicsEngine.h"
#include "Collision.h"

#include <algorithm>
#include "glmAddon.h"

#include "ECSComponents.h"


void applyImpulse(vec3 impulse, vec3 hitpos, PhysicsComponent& physicsComp, const TransformComponent& transform) {
    physicsComp.vel += impulse * physicsComp.invMass;

    vec3 rad = hitpos - transform.position;

    vec3 angularImpulse = cross(rad, impulse);
    vec3 localAngularImpulse = angularImpulse * transform.rotation;

    vec3 localAngVel = physicsComp.invInertia * localAngularImpulse;
    physicsComp.angVel += transform.rotation * localAngVel;
}

void applyAngularImpulse(vec3 angularImpulse, PhysicsComponent& physicsComp, const TransformComponent& transform) {
    vec3 localAngularImpulse = angularImpulse * transform.rotation;

    vec3 localAngVel = physicsComp.invInertia * localAngularImpulse;
    physicsComp.angVel += transform.rotation * localAngVel;
}

void applyPositionImpulse(vec3 impulse, vec3 hitPos, const PhysicsComponent& physicsComp, TransformComponent& transform) {
    transform.position += impulse * physicsComp.invMass;

    vec3 rad = hitPos - transform.position;

    vec3 rotImpulse = cross(rad, impulse);
    vec3 localRotImpulse = rotImpulse * transform.rotation;

    vec3 deltaRot = physicsComp.invInertia * localRotImpulse;

    vec3 w = deltaRot * 0.5f;
    float theta = length(w);
    if (theta > 0.f) {
        w = w / theta;
    }

    transform.rotation = transform.rotation * quat(cos(theta), sin(theta) * w);
}

// Depenetration through Pseudo Impulse
void PhysicsSolver::solvePosition(CollisionECS& collision) {
    ECS::ECSManager* ecs = physicsEngine->ecs;

    bool hasPhysicsA = ecs->hasComponent<PhysicsComponent>(collision.entityA);
    bool hasPhysicsB = ecs->hasComponent<PhysicsComponent>(collision.entityB);

    if (!hasPhysicsA && !hasPhysicsB) return;

    float biasSlop = physicsEngine->biasSlop;
    float biasFactor = physicsEngine->biasFactor;

    vec3 norm = collision.worldNormal;

    float effMass = 0.f;
    if (hasPhysicsA) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityA);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityA);

        vec3 rA = collision.pointA - transform.position;
        effMass += calcEffectiveMass(physics, transform, cross(-rA, norm));
    }
    if (hasPhysicsB) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityB);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityB);

        vec3 rB = collision.pointB - transform.position;
        effMass += calcEffectiveMass(physics, transform, cross(rB, norm));
    }

    // collision depth value isn't being updated upon sequential iterations
    float lambda = std::max(collision.depth - biasSlop, 0.f) * biasFactor / effMass;
    vec3 impulse = norm * lambda;

    if (hasPhysicsA) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityA);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityA);

        applyPositionImpulse(-impulse, collision.pointA, physics, transform);
    }
    if (hasPhysicsB) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityB);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityB);

        applyPositionImpulse(impulse, collision.pointB, physics, transform);
    }
}

// Normal Impulse
void PhysicsSolver::solveImpulse(CollisionECS& collision) {
    ECS::ECSManager* ecs = physicsEngine->ecs;

    bool hasPhysicsA = ecs->hasComponent<PhysicsComponent>(collision.entityA);
    bool hasPhysicsB = ecs->hasComponent<PhysicsComponent>(collision.entityB);

    if (!hasPhysicsA && !hasPhysicsB) return;

    vec3 norm = collision.worldNormal;

    float JV = 0.f;
    float effMass = 0.f;
    if (hasPhysicsA) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityA);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityA);

        vec3 rA = collision.pointA - transform.position;
        JV += calcRelativeVel(physics, rA, -norm);
        effMass += calcEffectiveMass(physics, transform, cross(-rA, norm));
    }
    if (hasPhysicsB) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityB);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityB);

        vec3 rB = collision.pointB - transform.position;
        JV += calcRelativeVel(physics, rB, norm);
        effMass += calcEffectiveMass(physics, transform, cross(rB, norm));
    }

    float lambda = -JV / effMass;
    float newSum = std::max(collision.lambdaSum + lambda, 0.f);
    lambda = newSum - collision.lambdaSum;
    collision.lambdaSum += lambda;

    vec3 impulse = norm * lambda;

    if (hasPhysicsA) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityA);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityA);

        applyImpulse(-impulse, collision.pointA, physics, transform);
    }
    if (hasPhysicsB) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityB);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityB);

        applyImpulse(impulse, collision.pointB, physics, transform);
    }
}


// Friction
void PhysicsSolver::solveFriction(CollisionECS& collision) {
    ECS::ECSManager* ecs = physicsEngine->ecs;

    bool hasPhysicsA = ecs->hasComponent<PhysicsComponent>(collision.entityA);
    bool hasPhysicsB = ecs->hasComponent<PhysicsComponent>(collision.entityB);

    if (!hasPhysicsA && !hasPhysicsB) return;

    vec3 norm = collision.worldNormal;
    vec3 tangent1 = { -norm.y, norm.x, 0.f };
    if (length(tangent1) == 0.f) {
        tangent1 = { 0.f, -norm.z, norm.y };
    }
    tangent1 = normalize(tangent1);
    vec3 tangent2 = cross(norm, tangent1);

    float maxFriction = collision.lambdaSum * physicsEngine->friction;

    float JV1 = 0.f;
    float effMass1 = 0.f;
    float JV2 = 0.f;
    float effMass2 = 0.f;
    if (hasPhysicsA) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityA);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityA);

        vec3 rA = collision.pointA - transform.position;
        JV1 += calcRelativeVel(physics, rA, -tangent1);
        effMass1 += calcEffectiveMass(physics, transform, cross(-rA, tangent1));
        JV2 += calcRelativeVel(physics, rA, -tangent2);
        effMass2 += calcEffectiveMass(physics, transform, cross(-rA, tangent2));
    }
    if (hasPhysicsB) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityB);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityB);

        vec3 rB = collision.pointB - transform.position;
        JV1 += calcRelativeVel(physics, rB, tangent1);
        effMass1 += calcEffectiveMass(physics, transform, cross(rB, tangent1));
        JV2 += calcRelativeVel(physics, rB, tangent2);
        effMass2 += calcEffectiveMass(physics, transform, cross(rB, tangent2));
    }

    float tangentLambda1 = -JV1 / effMass1;
    float newSum1 = std::clamp(collision.tangentLambdaSum1 + tangentLambda1, -maxFriction, maxFriction);
    tangentLambda1 = newSum1 - collision.tangentLambdaSum1;
    collision.tangentLambdaSum1 += tangentLambda1;

    float tangentLambda2 = -JV2 / effMass2;
    float newSum2 = std::clamp(collision.tangentLambdaSum2 + tangentLambda2, -maxFriction, maxFriction);
    tangentLambda2 = newSum2 - collision.tangentLambdaSum2;
    collision.tangentLambdaSum2 += tangentLambda2;

    vec3 frictionImpulse = tangent1 * tangentLambda1 + tangent2 * tangentLambda2;

    if (hasPhysicsA) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityA);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityA);

        applyImpulse(-frictionImpulse, collision.pointA, physics, transform);
    }
    if (hasPhysicsB) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityB);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityB);

        applyImpulse(frictionImpulse, collision.pointB, physics, transform);
    }
}

void PhysicsSolver::applyRestitution(CollisionECS& collision) {
    ECS::ECSManager* ecs = physicsEngine->ecs;

    bool hasPhysicsA = ecs->hasComponent<PhysicsComponent>(collision.entityA);
    bool hasPhysicsB = ecs->hasComponent<PhysicsComponent>(collision.entityB);

    if (!hasPhysicsA && !hasPhysicsB) return;

    vec3 norm = collision.worldNormal;
    float lambda = collision.lambdaSum;

    vec3 impulse = norm * lambda * physicsEngine->elasticity;

    if (hasPhysicsA) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityA);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityA);

        applyImpulse(-impulse, collision.pointA, physics, transform);
    }
    if (hasPhysicsB) {
        auto& physics = ecs->getComponent<PhysicsComponent>(collision.entityB);
        auto& transform = ecs->getComponent<TransformComponent>(collision.entityB);

        applyImpulse(impulse, collision.pointB, physics, transform);
    }
}


// Shorthand
//float PhysicsSolver::calcLambda(CollisionECS& collision) {
//    float JV = calcRelativeVel(collision);
//    float effMass = calcEffectiveMass(collision);
//    return -JV / effMass;
//}
//
//float PhysicsSolver::calcLambda(CollisionECS& collision, vec3 direction) {
//    float JV = calcRelativeVel(collision, direction);
//    float effMass = calcEffectiveMass(collision, direction);
//    return -JV / effMass;
//}


// Calculates Jacobian * Velocity
//float PhysicsSolver::calcRelativeVel(CollisionECS& collision) {
//    vec3 norm = collision.worldNormal;
//    return calcRelativeVel(collision, norm);
//}
//
//float PhysicsSolver::calcRelativeVel(const PhysicsComponent& physicsCompA, vec3 radA, const PhysicsComponent& physicsCompB, vec3 radB, vec3 direction) {
//    //PhysicsObject* A = collision.bodyA;
//    //PhysicsObject* B = collision.bodyB;
//
//
//    vec3 rA = radA;
//    float JVA = calcRelativeVel(A, rA, -direction);
//
//    vec3 rB = B ? collision.pointB - B->pos : vec3(0);
//    float JVB = B ? calcRelativeVel(B, rB, direction) : 0.f;
//
//    return JVA + JVB;
//}

float PhysicsSolver::calcRelativeVel(const PhysicsComponent& physicsComp, vec3 rad, vec3 direction) {
    vec3 Va = physicsComp.vel;
    vec3 Wa = physicsComp.angVel;

    float JV = dot(direction, Va) + dot(cross(rad, direction), Wa);
    return JV;
}


// Calculates effective mass of contact constraint
//float PhysicsSolver::calcEffectiveMass(CollisionECS& collision) {
//    vec3 norm = collision.worldNormal;
//    return calcEffectiveMass(collision, norm);
//}

//float PhysicsSolver::calcEffectiveMass(const PhysicsComponent& physicsA, const TransformComponent& transformA, const PhysicsComponent& physicsB, const TransformComponent& transformB, vec3 direction) {
//    //PhysicsObject* A = collision.bodyA;
//    //PhysicsObject* B = collision.bodyB;
//
//    vec3 rA = collision.pointA - transformA.position;
//    float effMassA = calcEffectiveMass(physicsA, transformA, cross(-rA, direction));
//
//
//
//    vec3 rB = B ? collision.pointB - B->pos : vec3(0);
//    float effMassB = B ? calcEffectiveMass(B, cross(rB, direction)) : 0.f;
//
//    return effMassA + effMassB;
//}

float PhysicsSolver::calcEffectiveMass(const PhysicsComponent& physics, const TransformComponent& transform, vec3 radNorm) {
    float iMa = physics.invMass;
    mat3 iIa = physics.invInertia;


    float effMass = iMa + dot(radNorm, transform.rotation * (iIa * radNorm * transform.rotation));
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