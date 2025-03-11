#pragma once

#include "PhysicsBody.h"

#include <glm/glm.hpp>

using glm::vec3;

struct Collision {
    PhysicsBody* bodyA = nullptr;
    PhysicsBody* bodyB = nullptr;

    vec3 worldNormal;
    vec3 pointA;
    vec3 pointB;
    float depth = 0.f;

    float lambdaSum = 0.f;
    float tangentLambdaSum = 0.f;
};

struct Joint {
    PhysicsBody* bodyA = nullptr;
    PhysicsBody* bodyB = nullptr;

    vec3 localA;
    vec3 localB;
};