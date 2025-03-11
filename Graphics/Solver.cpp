//#include "Solver.h"
//
//#include "Collision.h"
//
//#include <algorithm>
//
//
//// Normal Impulses
//void Solver::SolvePosition(Collision& collision) {
//    RigidBody* A = collision.bodyA;
//    RigidBody* B = collision.bodyB;
//
//    Vector2 norm = collision.worldNormal;
//
//    float invMassA = A->invMass;
//    float invMOIA = A->invMOI;
//
//    Vector2 radA = collision.pointA - A->pos;
//    Vector2 radPerpA = { -radA.y, radA.x };
//
//    float invMassB = 0.f;
//    float invMOIB = 0.f;
//
//    Vector2 radB = { 0.f, 0.f };
//    Vector2 radPerpB = { 0.f, 0.f };
//
//    if (B) {
//        invMassB = B->invMass;
//        invMOIB = B->invMOI;
//
//        radB = collision.pointB - B->pos;
//        radPerpB = { -radB.y, radB.x };
//    }
//
//    float effMass = invMassA + (Vector2DotProduct(norm * -1, radPerpA) * Vector2DotProduct(norm * -1, radPerpA) * invMOIA) + invMassB + (Vector2DotProduct(norm, radPerpB) * Vector2DotProduct(norm, radPerpB) * invMOIB);
//
//    float lambda = std::max(collision.depth - biasSlop, 0.f) * biasFactor / effMass;
//
//    A->pos += (norm * -invMassA) * lambda;
//    A->rot += Vector2DotProduct(norm * -1, radPerpA) * invMOIA * lambda;
//
//    if (B) {
//        B->pos += (norm * invMassB) * lambda;
//        B->rot += Vector2DotProduct(norm, radPerpB) * invMOIB * lambda;
//    }
//}
//
//void Solver::SolveImpulse(Collision& collision) {
//    RigidBody* A = collision.bodyA;
//    RigidBody* B = collision.bodyB;
//
//    Vector2 norm = collision.worldNormal;
//
//    Vector2 velA = A->vel;
//    float angVelA = A->angVel;
//
//    float invMassA = A->invMass;
//    float invMOIA = A->invMOI;
//
//    Vector2 radA = collision.pointA - A->pos;
//    Vector2 radPerpA = { -radA.y, radA.x };
//
//    Vector2 velB = { 0.f, 0.f };
//    float angVelB = 0.f;
//
//    float invMassB = 0.f;
//    float invMOIB = 0.f;
//
//    Vector2 radB = { 0.f, 0.f };
//    Vector2 radPerpB = { 0.f, 0.f };
//
//    if (B) {
//        velB = B->vel;
//        angVelB = B->angVel;
//
//        invMassB = B->invMass;
//        invMOIB = B->invMOI;
//
//        radB = collision.pointB - B->pos;
//        radPerpB = { -radB.y, radB.x };
//    }
//
//    float JV = Vector2DotProduct(norm * -1, velA) + (Vector2DotProduct(norm * -1, radPerpA) * angVelA) + Vector2DotProduct(norm, velB) + (Vector2DotProduct(norm, radPerpB) * angVelB);
//    float effMass = invMassA + (Vector2DotProduct(norm * -1, radPerpA) * Vector2DotProduct(norm * -1, radPerpA) * invMOIA) + invMassB + (Vector2DotProduct(norm, radPerpB) * Vector2DotProduct(norm, radPerpB) * invMOIB);
//
//    float lambda = -JV / effMass;
//    float newSum = std::max(collision.lambdaSum + lambda, 0.f);
//
//    lambda = newSum - collision.lambdaSum;
//    collision.lambdaSum += lambda;
//
//    A->ApplyImpulse(norm * -lambda, collision.pointA);
//    if (B) { B->ApplyImpulse(norm * lambda, collision.pointB); }
//}
//
//void Solver::SolveImpulsePair(Collision& collision1, Collision& collision2) {
//    RigidBody* A = collision1.bodyA;
//    RigidBody* B = collision1.bodyB;
//
//    Vector2 norm = collision1.worldNormal;
//
//    Vector2 velA = A->vel;
//    float angVelA = A->angVel;
//
//    float invMassA = A->invMass;
//    float invMOIA = A->invMOI;
//
//    Vector2 velB = { 0.f, 0.f };
//    float angVelB = 0.f;
//
//    float invMassB = 0.f;
//    float invMOIB = 0.f;
//
//    Vector2 radA = collision1.pointA - A->pos;
//    Vector2 radPerpA = { -radA.y, radA.x };
//
//    Vector2 radB = { 0.f, 0.f };
//    Vector2 radPerpB = { 0.f, 0.f };
//
//    Vector2 radC = collision2.pointA - A->pos;
//    Vector2 radPerpC = { -radC.y, radC.x };
//
//    Vector2 radD = { 0.f, 0.f };
//    Vector2 radPerpD = { 0.f, 0.f };
//
//    if (B) {
//        velB = B->vel;
//        angVelB = B->angVel;
//
//        invMassB = B->invMass;
//        invMOIB = B->invMOI;
//
//        radB = collision1.pointB - B->pos;
//        radPerpB = { -radB.y, radB.x };
//
//        radD = collision2.pointB - B->pos;
//        radPerpD = { -radD.y, radD.x };
//    }
//
//    float JV_1 = Vector2DotProduct(norm * -1, velA) + (Vector2DotProduct(norm * -1, radPerpA) * angVelA) + Vector2DotProduct(norm, velB) + (Vector2DotProduct(norm, radPerpB) * angVelB);
//    float JV_2 = Vector2DotProduct(norm * -1, velA) + (Vector2DotProduct(norm * -1, radPerpC) * angVelA) + Vector2DotProduct(norm, velB) + (Vector2DotProduct(norm, radPerpD) * angVelB);
//
//    float a = invMassA + (Vector2DotProduct(norm * -1, radPerpA) * Vector2DotProduct(norm * -1, radPerpA) * invMOIA) + invMassB + (Vector2DotProduct(norm, radPerpB) * Vector2DotProduct(norm, radPerpB) * invMOIB);
//    float b = invMassA + (Vector2DotProduct(norm * -1, radPerpC) * Vector2DotProduct(norm * -1, radPerpA) * invMOIA) + invMassB + (Vector2DotProduct(norm, radPerpD) * Vector2DotProduct(norm, radPerpB) * invMOIB);
//    float c = invMassA + (Vector2DotProduct(norm * -1, radPerpA) * Vector2DotProduct(norm * -1, radPerpC) * invMOIA) + invMassB + (Vector2DotProduct(norm, radPerpB) * Vector2DotProduct(norm, radPerpD) * invMOIB);
//    float d = invMassA + (Vector2DotProduct(norm * -1, radPerpC) * Vector2DotProduct(norm * -1, radPerpC) * invMOIA) + invMassB + (Vector2DotProduct(norm, radPerpD) * Vector2DotProduct(norm, radPerpD) * invMOIB);
//
//    float determinant = a * d - b * c;
//
//    a = a / determinant;
//    b = b / determinant;
//    c = c / determinant;
//    d = d / determinant;
//
//    float lambda1 = d * -JV_1 - b * -JV_2;
//    float lambda2 = -c * -JV_1 + a * -JV_2;
//
//    if (abs(lambda1) < 0.01f && abs(lambda2) < 0.01f) { return; }
//
//    float combinedSum = std::max(collision1.lambdaSum + collision2.lambdaSum + lambda1 + lambda2, 0.f);
//
//    lambda1 = combinedSum - collision1.lambdaSum - collision2.lambdaSum - lambda2;
//    lambda2 = combinedSum - collision1.lambdaSum - collision2.lambdaSum - lambda1;
//
//    collision1.lambdaSum += lambda1;
//    collision2.lambdaSum += lambda2;
//
//    A->ApplyImpulse(norm * -lambda1, collision1.pointA);
//    A->ApplyImpulse(norm * -lambda2, collision2.pointA);
//
//    if (B) {
//        B->ApplyImpulse(norm * lambda1, collision1.pointB);
//        B->ApplyImpulse(norm * lambda2, collision2.pointB);
//    }
//}
//
//
//// Friction
//void Solver::SolveFriction(Collision& collision) {
//    RigidBody* A = collision.bodyA;
//    RigidBody* B = collision.bodyB;
//
//    Vector2 norm = collision.worldNormal;
//    Vector2 tangent = { -norm.y, norm.x };
//
//    Vector2 velA = A->vel;
//    float angVelA = A->angVel;
//
//    float invMassA = A->invMass;
//    float invMOIA = A->invMOI;
//
//    Vector2 radA = collision.pointA - A->pos;
//    Vector2 radPerpA = { -radA.y, radA.x };
//
//    Vector2 velB = { 0.f, 0.f };
//    float angVelB = 0.f;
//
//    float invMassB = 0.f;
//    float invMOIB = 0.f;
//
//    Vector2 radB = { 0.f, 0.f };
//    Vector2 radPerpB = { 0.f, 0.f };
//
//    if (B) {
//        velB = B->vel;
//        angVelB = B->angVel;
//
//        invMassB = B->invMass;
//        invMOIB = B->invMOI;
//
//        radB = collision.pointB - B->pos;
//        radPerpB = { -radB.y, radB.x };
//    }
//
//    float JV = Vector2DotProduct(tangent * -1, velA) + (Vector2DotProduct(tangent * -1, radPerpA) * angVelA) + Vector2DotProduct(tangent, velB) + (Vector2DotProduct(tangent, radPerpB) * angVelB);
//    float effMass = invMassA + (Vector2DotProduct(tangent * -1, radPerpA) * Vector2DotProduct(tangent * -1, radPerpA) * invMOIA) + invMassB + (Vector2DotProduct(tangent, radPerpB) * Vector2DotProduct(tangent, radPerpB) * invMOIB);
//
//    float tangentLambda = -JV / effMass;
//
//    float maxFriction = collision.lambdaSum * friction;
//    float newSum = std::clamp(collision.tangentLambdaSum + tangentLambda, -maxFriction, maxFriction);
//
//    tangentLambda = newSum - collision.tangentLambdaSum;
//    collision.tangentLambdaSum += tangentLambda;
//
//    A->ApplyImpulse(tangent * -tangentLambda, collision.pointA);
//    if (B) { B->ApplyImpulse(tangent * tangentLambda, collision.pointB); }
//}
//
//void Solver::SolveFrictionPair(Collision& collision1, Collision& collision2) {
//    RigidBody* A = collision1.bodyA;
//    RigidBody* B = collision1.bodyB;
//
//    Vector2 norm = collision1.worldNormal;
//    Vector2 tangent = { -norm.y, norm.x };
//
//    Vector2 velA = A->vel;
//    float angVelA = A->angVel;
//
//    float invMassA = A->invMass;
//    float invMOIA = A->invMOI;
//
//    Vector2 velB = { 0.f, 0.f };
//    float angVelB = 0.f;
//
//    float invMassB = 0.f;
//    float invMOIB = 0.f;
//
//    Vector2 radA = collision1.pointA - A->pos;
//    Vector2 radPerpA = { -radA.y, radA.x };
//
//    Vector2 radB = { 0.f, 0.f };
//    Vector2 radPerpB = { 0.f, 0.f };
//
//    Vector2 radC = collision2.pointA - A->pos;
//    Vector2 radPerpC = { -radC.y, radC.x };
//
//    Vector2 radD = { 0.f, 0.f };
//    Vector2 radPerpD = { 0.f, 0.f };
//
//    if (B) {
//        velB = B->vel;
//        angVelB = B->angVel;
//
//        invMassB = B->invMass;
//        invMOIB = B->invMOI;
//
//        radB = collision1.pointB - B->pos;
//        radPerpB = { -radB.y, radB.x };
//
//        radD = collision2.pointB - B->pos;
//        radPerpD = { -radD.y, radD.x };
//    }
//
//    float JV_1 = Vector2DotProduct(tangent * -1, velA) + (Vector2DotProduct(tangent * -1, radPerpA) * angVelA) + Vector2DotProduct(tangent, velB) + (Vector2DotProduct(tangent, radPerpB) * angVelB);
//    float JV_2 = Vector2DotProduct(tangent * -1, velA) + (Vector2DotProduct(tangent * -1, radPerpC) * angVelA) + Vector2DotProduct(tangent, velB) + (Vector2DotProduct(tangent, radPerpD) * angVelB);
//
//    float effMass1 = invMassA + (Vector2DotProduct(tangent * -1, radPerpA) * Vector2DotProduct(tangent * -1, radPerpA) * invMOIA) + invMassB + (Vector2DotProduct(tangent, radPerpB) * Vector2DotProduct(tangent, radPerpB) * invMOIB);
//    float effMass2 = invMassA + (Vector2DotProduct(tangent * -1, radPerpC) * Vector2DotProduct(tangent * -1, radPerpC) * invMOIA) + invMassB + (Vector2DotProduct(tangent, radPerpD) * Vector2DotProduct(tangent, radPerpD) * invMOIB);
//
//    float tangentLambda1 = -JV_1 / effMass1;
//    float tangentLambda2 = -JV_2 / effMass2;
//
//    if (abs(tangentLambda1) < 0.01f && abs(tangentLambda2) < 0.01f) { return; }
//
//    float maxFriction = abs(collision1.lambdaSum + collision2.lambdaSum) * friction * 0.5;
//
//    float combinedSum1 = std::clamp(collision1.tangentLambdaSum + tangentLambda1, -maxFriction, maxFriction);
//    float combinedSum2 = std::clamp(collision2.tangentLambdaSum + tangentLambda2, -maxFriction, maxFriction);
//
//    tangentLambda1 = combinedSum1 - collision1.tangentLambdaSum;
//    tangentLambda2 = combinedSum2 - collision2.tangentLambdaSum;
//
//    collision1.tangentLambdaSum += tangentLambda1;
//    collision2.tangentLambdaSum += tangentLambda2;
//
//    A->ApplyImpulse(tangent * -tangentLambda1, collision1.pointA);
//    A->ApplyImpulse(tangent * -tangentLambda2, collision2.pointA);
//
//    if (B) {
//        B->ApplyImpulse(tangent * tangentLambda1, collision1.pointB);
//        B->ApplyImpulse(tangent * tangentLambda2, collision2.pointB);
//    }
//}
//
//// Joints
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
//
//void Solver::ApplyRestitution(Collision& collision) {
//    Vector2 norm = collision.worldNormal;
//    float lambda = collision.lambdaSum;
//
//    RigidBody* A = collision.bodyA;
//    RigidBody* B = collision.bodyB;
//
//    A->ApplyImpulse(norm * -lambda * elasticity, collision.pointA);
//    if (B) { B->ApplyImpulse(norm * lambda * elasticity, collision.pointB); }
//}