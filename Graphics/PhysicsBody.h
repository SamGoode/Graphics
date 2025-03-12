#pragma once

#include "Gizmos.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::quat;

class PhysicsBody {
public:
    bool isStatic;

    vec3 pos = vec3(0);
    vec3 vel = vec3(0);
    vec3 acc = vec3(0);
    float invMass;

    vec3 rot = vec3(0);
    vec3 angVel = vec3(0);
    vec3 angAcc = vec3(0);
    mat3 invInertia = mat3(0);

    vec4 color = vec4(0, 0, 0, 1);

public:
    PhysicsBody() {}
    PhysicsBody(vec3 _pos, float _mass);

    PhysicsBody* setColor(vec4 _color) { color = _color; return this; }

    void applyImpulse(vec3 impulse, vec3 hitPos);
    void applyAngularImpulse(vec3 angularImpulse);

    void update(float deltaTime);
    void finaliseUpdate(float deltaTime);

    virtual int getID() = 0;
    virtual void draw() = 0;
};

class Box : public PhysicsBody {
public:
    vec3 extents;

public:
    Box() {}
    Box(vec3 _pos, float _mass, vec3 _extents, vec3 _rotation);

    virtual int getID() override { return 0; };
    virtual void draw() override;
};

class Sphere : public PhysicsBody {
public:
    float radius;

public:
    Sphere() {}
    Sphere(vec3 _pos, float _mass, float _radius);

    virtual int getID() override { return 1; };
    virtual void draw() override;
};


struct Plane {
    vec3 normal;
    float distanceFromOrigin;

    bool isPointUnderPlane(vec3 point) {
        return dot(point, normal) < -distanceFromOrigin;
    }
};