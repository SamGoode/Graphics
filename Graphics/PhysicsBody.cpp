#include "PhysicsBody.h"

#include "Gizmos.h"

using aie::Gizmos;

using glm::quat;


PhysicsBody::PhysicsBody(vec3 _pos, float _mass) {
    isStatic = _mass <= 0.f;

    pos = _pos;
    invMass = isStatic ? 0.f : (1 / _mass);
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
    rot += ((quat(0, angVel) * rot) * (deltaTime * 0.5f));
    normalize(rot);
    angAcc = vec3(0);
}

void PhysicsBody::finaliseUpdate(float DeltaTime) {
    if (isStatic) { return; }

    vel += acc * DeltaTime * 0.5f;
    angVel += angAcc * DeltaTime * 0.5f;
}


Box::Box(vec3 _pos, float _mass, vec3 _extents, vec3 _eulerRotation) : PhysicsBody(_pos, _mass) {
    vec3 euler = glm::radians(_eulerRotation);
    quat qx = quat(cos(euler.x / 2), sin(euler.x / 2) * vec3(1, 0, 0));
    quat qy = quat(cos(euler.y / 2), sin(euler.y / 2) * vec3(0, 1, 0));
    quat qz = quat(cos(euler.z / 2), sin(euler.z / 2) * vec3(0, 0, 1));
    rot = (qx * qy * qz);
    //rot = rot * qy;
    //rot = qz * rot;
    //rot = rot * qx;

    vec3 dimensions = _extents * 2.f;

    if (_mass > 0.f) {
        invInertia[0][0] = 12.f / (_mass * (dimensions.y * dimensions.y + dimensions.z * dimensions.z));
        invInertia[1][1] = 12.f / (_mass * (dimensions.x * dimensions.x + dimensions.z * dimensions.z));
        invInertia[2][2] = 12.f / (_mass * (dimensions.x * dimensions.x + dimensions.y * dimensions.y));
    }

    extents = _extents;
}

void Box::draw() {
    // Gizmos treats y as the up axis
    //quat q = quatLookAt(vec3(-1, 0, 0), vec3(0, 0, 1));
    //q = rotate(q, rot.x, vec3(1, 0, 0));
    //q = rotate(q, rot.y, vec3(0, 0, 1));
    //q = rotate(q, rot.z, vec3(0, 1, 0));

    mat4 rotMat = glm::mat4_cast(rot);
    //mat4 rotMat = glm::identity<mat4>();

    Gizmos::addAABBFilled(pos, extents, color, &rotMat);
}


Sphere::Sphere(vec3 _pos, float _mass, float _radius) : PhysicsBody(_pos, _mass) {
    if (_mass > 0.f) {
        invInertia[0][0] = (2 * _mass * _radius * _radius) / 5.f;
        invInertia[1][1] = (2 * _mass * _radius * _radius) / 5.f;
        invInertia[2][2] = (2 * _mass * _radius * _radius) / 5.f;
    }

    radius = _radius;
}

void Sphere::draw() {
    Gizmos::addSphere(pos, radius, 10, 10, color);
}


