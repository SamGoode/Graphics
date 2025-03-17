#pragma once


struct Detector {
private:
    struct IPhysicsEngine* physEngInterface;

    typedef void (Detector::*func)(class PhysicsObject* A, PhysicsObject* B);
    func f[3] = {
        &Detector::checkCollision00,
        &Detector::checkCollision01,
        &Detector::checkCollision11
    };

public:
    Detector(IPhysicsEngine* interface) : physEngInterface(interface) {}

    void checkCollision(PhysicsObject* body, struct Plane plane);
    void checkCollision(PhysicsObject* A, PhysicsObject* B);

private:
    void checkCollision00(PhysicsObject* boxA, PhysicsObject* boxB);
    void checkCollision01(PhysicsObject* box, PhysicsObject* sphere);
    void checkCollision11(PhysicsObject* sphereA, PhysicsObject* sphereB);
};