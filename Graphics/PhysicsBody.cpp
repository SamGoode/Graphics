#include "PhysicsBody.h"

#include "Gizmos.h"

using aie::Gizmos;

//void RigidBody::ApplyImpulse(Vect impulse, Vector2 hitPos) {
//    Vector2 rad = hitPos - pos;
//    Vector2 radPerp = { -rad.y, rad.x };
//
//    vel += impulse * invMass;
//    angVel += Vector2DotProduct(impulse, radPerp) * invMOI;
//}
//
//void RigidBody::ApplyAngularImpulse(float angularImpulse) {
//    angVel += angularImpulse * invMOI;
//}

void PhysicsBody::update(float deltaTime) {
    if (isStatic) { return; }

    vel += acc * deltaTime * 0.5f;
    pos += vel * deltaTime;
    acc = vec3(0);

    angVel += angAcc * deltaTime * 0.5f;
    rot += angVel * deltaTime;
    angAcc = vec3(0);
}

//void RigidBody::PrepUpdate(float DeltaTime) {
//    if (isStatic) { return; }
//
//    vel += acc * DeltaTime * 0.5;
//    angVel += angAcc * DeltaTime * 0.5;
//}


//RigidRect::RigidRect(Vector2 _pos, float _mass, float _width, float _height, float _rotation) {
//    isStatic = (_mass <= 0.f);
//    
//    pos = _pos;
//    vel = { 0.f, 0.f };
//    acc = { 0.f, 0.f };
//    invMass = isStatic ? 0.f : (1.f / _mass);
//
//    rot = _rotation;
//    angVel = 0.f;
//    angAcc = 0.f;
//    invMOI = isStatic ? 0.f : 12.f / (_mass * (_width * _width + _height * _height));
//
//    width = _width;
//    height = _height;
//}
//
//void RigidRect::Draw(float scale) {
//    //Color color = BLUE;
//    if (isStatic) { color = GREEN; }
//    DrawRectanglePro({ pos.x * scale, pos.y * scale, width * scale, height * scale }, { width * scale / 2, height * scale / 2 }, rot * 180 / PI, color);
//}


Sphere::Sphere(vec3 _pos, float _mass, float _radius) {
    isStatic = (_mass <= 0.f);
    
    pos = _pos;
    invMass = isStatic ? 0.f : (1.f / _mass);

    //invMOI = isStatic ? 0.f : 1 / (_mass * _radius * _radius * 0.5f);

    radius = _radius;
}



void Sphere::draw() {
    Gizmos::addSphere(pos, radius, 10, 10, color);
}


