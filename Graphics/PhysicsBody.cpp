#include "PhysicsBody.h"

#include "Gizmos.h"
#include "glmAddon.h"


using aie::Gizmos;


GameObject::GameObject(vec3 _pos, vec3 _eulerRot, Geometry* _geometry) {
    pos = _pos;
    
    vec3 euler = glm::radians(_eulerRot);
    quat qx = quat(cos(euler.x / 2), sin(euler.x / 2) * vec3(1, 0, 0));
    quat qy = quat(cos(euler.y / 2), sin(euler.y / 2) * vec3(0, 1, 0));
    quat qz = quat(cos(euler.z / 2), sin(euler.z / 2) * vec3(0, 0, 1));
    rot = normalize(qx * qy * qz);

    shape = _geometry;
}


PhysicsBody::PhysicsBody(vec3 _pos, vec3 _eulerRot, Geometry* _geometry, float _mass) : GameObject(_pos, _eulerRot, _geometry) {
    isStatic = _mass <= 0.f;

    invMass = isStatic ? 0.f : (1 / _mass);
    invInertia = isStatic ? mat3(0) : shape->calculateInertiaTensor(_mass);
}


void PhysicsBody::applyImpulse(vec3 impulse, vec3 hitPos) {
    vel += impulse * invMass;

    vec3 rad = hitPos - pos;

    vec3 angularImpulse = cross(rad, impulse);
    vec3 localAngularImpulse = angularImpulse * rot;

    vec3 localAngVel = invInertia * localAngularImpulse;
    angVel += rot * localAngVel;
}

void PhysicsBody::applyAngularImpulse(vec3 angularImpulse) {
    vec3 localAngularImpulse = angularImpulse * rot;

    vec3 localAngVel = invInertia * localAngularImpulse;
    angVel += rot * localAngVel;
}



void PhysicsBody::update(float deltaTime) {
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

void PhysicsBody::finaliseUpdate(float DeltaTime) {
    if (isStatic) { return; }

    vel += acc * DeltaTime * 0.5f;
    angVel += angAcc * DeltaTime * 0.5f;
}


