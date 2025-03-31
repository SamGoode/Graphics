#pragma once

#include <glm/glm.hpp>
#include "ECS.h"

using glm::vec3;


struct Collision {
    class PhysicsObject* bodyA = nullptr;
    PhysicsObject* bodyB = nullptr;

    vec3 worldNormal;
    vec3 pointA;
    vec3 pointB;
    float depth = 0.f;

    float lambdaSum = 0.f;
    float tangentLambdaSum1 = 0.f;
    float tangentLambdaSum2 = 0.f;
};

struct CollisionECS {
    ECS::uint entityA;
    ECS::uint entityB;

    vec3 worldNormal;
    vec3 pointA;
    vec3 pointB;
    float depth = 0.f;

    float lambdaSum = 0.f;
    float tangentLambdaSum1 = 0.f;
    float tangentLambdaSum2 = 0.f;
};

struct Joint {
    PhysicsObject* bodyA = nullptr;
    PhysicsObject* bodyB = nullptr;

    vec3 localA;
    vec3 localB;
};