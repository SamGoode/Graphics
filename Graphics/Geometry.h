#pragma once

#include <glm/glm.hpp>
#include "Gizmos.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::quat;

using aie::Gizmos;


class Geometry {
public:
    virtual ~Geometry() {}
    virtual int getID() = 0;
    virtual void draw(vec3 pos, quat orientation, vec4 color) = 0;
    virtual mat3 calculateInertiaTensor(float mass) = 0;
};


class Box : public Geometry {
public:
    vec3 extents;

public:
    Box(float width, float height, float depth) : extents(width / 2, height / 2, depth / 2) {}

    virtual int getID() override { return 0; }
    virtual mat3 calculateInertiaTensor(float mass) override {
        vec3 dimensions = extents * 2.f;
        mat3 invInertia = mat3(0);
        invInertia[0][0] = 12.f / (mass * (dimensions.y * dimensions.y + dimensions.z * dimensions.z));
        invInertia[1][1] = 12.f / (mass * (dimensions.x * dimensions.x + dimensions.z * dimensions.z));
        invInertia[2][2] = 12.f / (mass * (dimensions.x * dimensions.x + dimensions.y * dimensions.y));

        return invInertia;
    }

    virtual void draw(vec3 pos, quat orientation, vec4 color) override {
        mat4 rotMat = glm::mat4_cast(orientation);
        Gizmos::addAABBFilled(pos, extents, color, &rotMat);
    }
};


class Sphere : public Geometry {
public:
    float radius;

public:
    Sphere(float _radius) : radius(_radius) {}

    virtual int getID() override { return 1; }
    virtual mat3 calculateInertiaTensor(float mass) override {
        mat3 invInertia = glm::identity<mat3>();
        invInertia *= (2 * mass * radius * radius) / 5.f;

        return invInertia;
    }

    virtual void draw(vec3 pos, quat orientation, vec4 color) override {
        mat4 rotMat = glm::mat4_cast(orientation);
        Gizmos::addSphere(pos, radius, 10, 10, color, &rotMat);
    }
};