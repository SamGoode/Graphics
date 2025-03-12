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
    vec3 rad = hitPos - pos;

    vel += impulse * invMass;
    quat q(rot);
    
    quat angularImpulse = quat(cross(rad, impulse));
    quat localAngularImpulse = q * angularImpulse;

    vec3 angles = glm::eulerAngles(localAngularImpulse);
    vec3 localAngImpulse = vec3(angles.y, angles.x, angles.z);

    vec3 localAngVel = invInertia * localAngImpulse;

    vec3 deltaAngVel = glm::eulerAngles(quat(q.w, -q.x, -q.y, -q.z) * quat(localAngVel));

    angVel += vec3(deltaAngVel.y, deltaAngVel.x, deltaAngVel.z);

    //vec3 localImpulse = quat(q.w, -q.x, -q.y, -q.z) * impulse;
    //vec3 localRad = quat(q.w, -q.x, -q.y, -q.z) * rad;
    //vec3 localAngVel = invInertia * cross(localRad, localImpulse);
    
    



    //angVel += q * localAngVel;

    float test = 8;
}

void PhysicsBody::applyAngularImpulse(vec3 angularImpulse) {
    //angVel += invMOI * angularImpulse;
}

void PhysicsBody::update(float deltaTime) {
    if (isStatic) { return; }

    vel += acc * deltaTime * 0.5f;
    pos += vel * deltaTime;
    acc = vec3(0);

    angVel += angAcc * deltaTime * 0.5f;
    rot += angVel * deltaTime;
    angAcc = vec3(0);
}

void PhysicsBody::finaliseUpdate(float DeltaTime) {
    if (isStatic) { return; }

    vel += acc * DeltaTime * 0.5f;
    angVel += angAcc * DeltaTime * 0.5f;
}


Box::Box(vec3 _pos, float _mass, vec3 _extents, vec3 _rotation) : PhysicsBody(_pos, _mass) {
    rot = _rotation;

    vec3 dimensions = _extents * 0.5f;

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

    mat4 rotMat = glm::mat4_cast(quat(rot));

    Gizmos::addAABBFilled(pos, vec3(extents.x, extents.y, extents.z), color, &rotMat);
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


