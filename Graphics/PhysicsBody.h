#pragma once

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Geometry.h"
#include "registry.h"

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::quat;


class GameObject : public registry<GameObject> {
public:
    vec3 pos = vec3(0);
    quat rot = quat(1, 0, 0, 0);

    vec4 color = vec4(0, 0, 0, 1);
    Geometry* shape = nullptr;

public:
    GameObject() {}
    GameObject(vec3 _pos, vec3 _eulerRot, Geometry* _geometry);
    virtual ~GameObject() { delete shape; }

    virtual void draw() { shape->draw(pos, rot, color); }
    void setColor(vec4 _color) { color = _color; }

    int getID() { return shape->getID(); };
};


class PhysicsBody : public GameObject, public registry<PhysicsBody> {
public:
    bool isStatic;

    vec3 vel = vec3(0);
    vec3 acc = vec3(0);
    float invMass = 0.f;

    vec3 angVel = vec3(0);
    vec3 angAcc = vec3(0);
    mat3 invInertia = mat3(0);

public:
    PhysicsBody() {}
    PhysicsBody(vec3 _pos, vec3 _eulerRot, Geometry* _geometry, float _mass);
    virtual ~PhysicsBody() {}

    void applyImpulse(vec3 impulse, vec3 hitPos);
    void applyAngularImpulse(vec3 angularImpulse);

    void update(float deltaTime);
    void finaliseUpdate(float deltaTime);
};


struct Plane {
    vec3 normal;
    float distanceFromOrigin;

    bool isPointUnderPlane(vec3 point) {
        return dot(point, normal) < -distanceFromOrigin;
    }
};