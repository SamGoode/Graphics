#pragma once

#include <glm/glm.hpp>
#include "ECS.h"


struct CollisionECS {
    ECS::uint entityA = UINT8_MAX;
    ECS::uint entityB = UINT8_MAX;

    glm::vec3 worldNormal;
    glm::vec3 pointA;
    glm::vec3 pointB;
    float depth = 0.f;

    float lambdaSum = 0.f;
    float tangentLambdaSum1 = 0.f;
    float tangentLambdaSum2 = 0.f;
};

//struct Joint {
//    PhysicsObject* bodyA = nullptr;
//    PhysicsObject* bodyB = nullptr;
//
//    vec3 localA;
//    vec3 localB;
//};