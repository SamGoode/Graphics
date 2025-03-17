#include "PhysicsBody.h"

#include "Gizmos.h"
#include "glmAddon.h"


using aie::Gizmos;


PhysicsObject::PhysicsObject(vec3 _pos, vec3 _eulerRot, Geometry* _geometry, float _mass) : RenderObject(_pos, _eulerRot, _geometry) {
    isStatic = _mass <= 0.f;

    invMass = isStatic ? 0.f : (1 / _mass);
    invInertia = isStatic ? mat3(0) : shape->calculateInertiaTensor(_mass);
}


void PhysicsObject::applyImpulse(vec3 impulse, vec3 hitPos) {
    vel += impulse * invMass;

    vec3 rad = hitPos - pos;

    vec3 angularImpulse = cross(rad, impulse);
    vec3 localAngularImpulse = angularImpulse * rot;

    vec3 localAngVel = invInertia * localAngularImpulse;
    angVel += rot * localAngVel;
}

void PhysicsObject::applyAngularImpulse(vec3 angularImpulse) {
    vec3 localAngularImpulse = angularImpulse * rot;

    vec3 localAngVel = invInertia * localAngularImpulse;
    angVel += rot * localAngVel;
}

void PhysicsObject::applyPositionImpulse(vec3 impulse, vec3 hitPos) {
    pos += impulse * invMass;

    vec3 rad = hitPos - pos;

    vec3 rotImpulse = cross(rad, impulse);
    vec3 localRotImpulse = rotImpulse * rot;

    vec3 deltaRot = invInertia * localRotImpulse;

    vec3 w = deltaRot;
    float theta = length(w);
    if (theta > 0.f) {
        w = w / theta;
    }
    
    rot = quat(cos(theta), sin(theta) * w) * rot;
}

void PhysicsObject::update(float deltaTime) {
    if (isStatic) { return; }

    vel += acc * deltaTime * 0.5f;
    pos += vel * deltaTime;
    acc = vec3(0);

    angVel += angAcc * deltaTime * 0.5f;

    if (length(angVel) < 2.f) { 
        rot += quat(0, angVel * (deltaTime * 0.5f)) * rot; // Approximation
    }
    else {
        rot = deltaRotation(angVel, deltaTime) * rot;
    }

    rot = normalize(rot);
    angAcc = vec3(0);
}

void PhysicsObject::finaliseUpdate(float DeltaTime) {
    if (isStatic) { return; }

    vel += acc * DeltaTime * 0.5f;
    angVel += angAcc * DeltaTime * 0.5f;
}


