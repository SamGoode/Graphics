#include "PhysicsBody.h"

#include "Gizmos.h"

using aie::Gizmos;

using glm::quat;




GameObject::GameObject(vec3 _pos, vec3 _eulerRot, Geometry* _geometry) {
    pos = _pos;
    
    vec3 euler = glm::radians(_eulerRot);
    quat qx = quat(cos(euler.x / 2), sin(euler.x / 2) * vec3(1, 0, 0));
    quat qy = quat(cos(euler.y / 2), sin(euler.y / 2) * vec3(0, 1, 0));
    quat qz = quat(cos(euler.z / 2), sin(euler.z / 2) * vec3(0, 0, 1));
    rot = normalize(qx * qy * qz);

    geometry = _geometry;
}

void GameObject::draw() {
    mat4 rotMat = glm::mat4_cast(rot);

    switch (geometry->getID()) {
    case 0:
    {
        vec3 extents = dynamic_cast<Box*>(geometry)->extents;
        Gizmos::addAABBFilled(pos, extents, color, &rotMat);
        break;
    }
    case 1:
    {
        Sphere* sphere = dynamic_cast<Sphere*>(geometry);
        Gizmos::addSphere(pos, sphere->radius, 10, 10, color, &rotMat);
        break;
    }
    }
}



PhysicsBody::PhysicsBody(vec3 _pos, vec3 _eulerRot, Geometry* _geometry, float _mass) : GameObject(_pos, _eulerRot, _geometry) {
    isStatic = _mass <= 0.f;

    invMass = isStatic ? 0.f : (1 / _mass);

    if (!isStatic) {
        switch (geometry->getID()) {
        case 0:
        {
            Box* box = dynamic_cast<Box*>(geometry);

            vec3 dimensions = box->extents * 2.f;
            invInertia[0][0] = 12.f / (_mass * (dimensions.y * dimensions.y + dimensions.z * dimensions.z));
            invInertia[1][1] = 12.f / (_mass * (dimensions.x * dimensions.x + dimensions.z * dimensions.z));
            invInertia[2][2] = 12.f / (_mass * (dimensions.x * dimensions.x + dimensions.y * dimensions.y));
            break;
        }
        case 1:
        {
            Sphere* sphere = dynamic_cast<Sphere*>(geometry);

            float radius = sphere->radius;
            invInertia = glm::identity<mat4>();
            invInertia *= (2 * _mass * radius * radius) / 5.f;
            break;
        }
        }
    }
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

quat deltaRotation(vec3 angularVelocity, float deltaTime) {
    vec3 w = angularVelocity * (deltaTime * 0.5f);
    float theta = length(w);
    if (theta > 0.f) {
        w = w / theta;
    }
    return quat(cos(theta), sin(theta) * w);
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


//Box::Box(float _mass, vec3 _pos, vec3 _eulerRotation, vec3 _extents) : PhysicsBody(_mass, _pos, _eulerRotation) {
//    if (_mass > 0.f) {
//        vec3 dimensions = _extents * 2.f;
//        invInertia[0][0] = 12.f / (_mass * (dimensions.y * dimensions.y + dimensions.z * dimensions.z));
//        invInertia[1][1] = 12.f / (_mass * (dimensions.x * dimensions.x + dimensions.z * dimensions.z));
//        invInertia[2][2] = 12.f / (_mass * (dimensions.x * dimensions.x + dimensions.y * dimensions.y));
//    }
//
//    extents = _extents;
//}

//void Box::draw() {
//    mat4 rotMat = glm::mat4_cast(rot);
//    Gizmos::addAABBFilled(pos, extents, color, &rotMat);
//}


//Sphere::Sphere(float _mass, vec3 _pos, vec3 _eulerRot, float _radius) : PhysicsBody(_mass, _pos, _eulerRot) {
//    if (_mass > 0.f) {
//        invInertia = glm::identity<mat4>();
//        invInertia *= (2 * _mass * _radius * _radius) / 5.f;
//    }
//
//    radius = _radius;
//}

//void Sphere::draw() {
//    mat4 rotMat = glm::mat4_cast(rot);
//    Gizmos::addSphere(pos, radius, 10, 10, color, &rotMat);
//}


