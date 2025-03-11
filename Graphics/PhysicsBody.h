#pragma once

#include <glm/glm.hpp>

using glm::vec3;
using glm::vec4;


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
    //vec3 invMOI = vec3(0);

    vec4 color = vec4(0, 0, 0, 1);

public:
    PhysicsBody() {}

    void setColor(vec4 _color) { color = _color; }

    //void applyImpulse(vec3 impulse, vec3 hitPos);
    //void ApplyAngularImpulse(float angularImpulse);

    void update(float deltaTime);
    //void prepUpdate(float deltaTime);

    virtual int getID() = 0;
    virtual void draw() = 0;
};


//class Box : public PhysicsBody {
//public:
//    vec3 m_extents;
//
//public:
//    Box() {}
//    Box(vec3 pos, float mass, vec3 extents, float rotation);
//
//    virtual int GetID() override { return 0; };
//    virtual void Draw(float scale) override;
//};


class Sphere : public PhysicsBody {
public:
    float radius;

public:
    Sphere() {}
    Sphere(vec3 _pos, float _mass, float _radius);

    virtual int getID() override { return 1; };
    virtual void draw() override;
};

