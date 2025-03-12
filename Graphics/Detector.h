#pragma once

#include "Collision.h"

#define MAX_COLLISIONS 64

class Detector {
public:
    int collisionCount = 0;
    const int maxCollisions = MAX_COLLISIONS;
    Collision collisions[MAX_COLLISIONS];

public:

    //void CheckBoundaryCollision(class PhysicsBody* body, Vector4 boundary);

    //void CheckCollision(class PhysicsBody* A, PhysicsBody* B);
    //void CheckCollisionRR(PhysicsBody* A, PhysicsBody* B);
    //void CheckCollisionRC(PhysicsBody* A, PhysicsBody* B);
    //void CheckCollisionCC(PhysicsBody* A, PhysicsBody* B);

    //void AddCollision(Collision collision) { collisions[collisionCount++] = collision; }
    //void ClearCollisions() { collisionCount = 0; }

    //bool IsWithinBody(PhysicsBody* body, Vector2 pos);
};