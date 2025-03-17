#pragma once

#include "GameObject.h"
#include "RenderObject.h"


//class CollisionObject : public RenderObject, public Registry<RenderObject> {
//public:
//    virtual int getID() { return shape->getID(); };
//};

class PhysicsObject : public RenderObject, public Registry<PhysicsObject> {
public:
    bool isStatic;

    vec3 vel = vec3(0);
    vec3 acc = vec3(0);
    float invMass = 0.f;

    vec3 angVel = vec3(0);
    vec3 angAcc = vec3(0);
    mat3 invInertia = mat3(0);

public:
    PhysicsObject() {}
    PhysicsObject(vec3 _pos, vec3 _eulerRot, Geometry* _geometry, float _mass);
    virtual ~PhysicsObject() {}

    void applyImpulse(vec3 impulse, vec3 hitPos);
    void applyAngularImpulse(vec3 angularImpulse);

    void applyPositionImpulse(vec3 impulse, vec3 hitPos);

    virtual void update(float deltaTime);
    virtual void finaliseUpdate(float deltaTime);
};


struct Plane {
    vec3 normal;
    float distanceFromOrigin;

    bool isPointUnderPlane(vec3 point) {
        return dot(point, normal) < -distanceFromOrigin;
    }
};