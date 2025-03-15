#pragma once

#include "Gizmos.h"
#include <glm/glm.hpp>
#include <glm/ext.hpp>

using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::quat;


class Geometry {
public:
    virtual ~Geometry() {}
    virtual int getID() = 0;
};

class Box : public Geometry {
public:
    vec3 extents;

public:
    Box(float width, float height, float depth) : extents(width/2, height/2, depth/2) {}
    virtual int getID() override { return 0; }
};

class Sphere : public Geometry {
public:
    float radius;

public:
    Sphere(float _radius) : radius(_radius) {}
    virtual int getID() override { return 1; }
};


class GameObject {
public:
    vec3 pos = vec3(0);
    quat rot = quat(1, 0, 0, 0);

    vec4 color = vec4(0, 0, 0, 1);

    Geometry* geometry = nullptr;

public:
    GameObject() {}
    GameObject(vec3 _pos, vec3 _eulerRot, Geometry* _geometry);
    virtual ~GameObject() { delete geometry; }

    int getID() { return geometry->getID(); };
    virtual void draw();

    void setColor(vec4 _color) { color = _color; }
};

class PhysicsBody : public GameObject {
public:
    bool isStatic;

    vec3 vel = vec3(0);
    vec3 acc = vec3(0);
    float invMass;

    vec3 angVel = vec3(0);
    vec3 angAcc = vec3(0);
    mat3 invInertia = mat3(0);

public:
    PhysicsBody() {}
    PhysicsBody(vec3 _pos, vec3 _eulerRot, Geometry* _geometry, float _mass);
    virtual ~PhysicsBody() {}

    //PhysicsBody* setColor(vec4 _color) { color = _color; return this; }

    void applyImpulse(vec3 impulse, vec3 hitPos);
    void applyAngularImpulse(vec3 angularImpulse);

    void update(float deltaTime);
    void finaliseUpdate(float deltaTime);
};



//class Box : public GameObject {
//public:
//    vec3 extents;
//
//public:
//    Box() {}
//    Box(float _mass, vec3 _pos, vec3 _eulerRot, vec3 _extents);
//
//    virtual int getID() override { return 0; };
//    virtual void draw() override;
//};
//
//class Sphere : public PhysicsBody {
//public:
//    float radius;
//
//public:
//    Sphere() {}
//    Sphere(float _mass, vec3 _pos, vec3 _eulerRot, float _radius);
//
//    virtual int getID() override { return 1; };
//    virtual void draw() override;
//};









struct Plane {
    vec3 normal;
    float distanceFromOrigin;

    bool isPointUnderPlane(vec3 point) {
        return dot(point, normal) < -distanceFromOrigin;
    }
};