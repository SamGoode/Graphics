#pragma once

struct Collision;

class Solver {
public:
    int iterations = 4;

    float biasSlop = 0.005f;
    float biasFactor = 0.1f;

    float elasticity = 0.2f;
    float friction = 0.95f;

public:
    //Solver(int _iterations, float _biasSlop, float _biasFactor, float _elasticity, float _friction);

    //void SolvePosition(struct Collision& collision);
    void SolveImpulse(Collision& collision);
    //void SolveImpulsePair(Collision& collision1, Collision& collision2);

    //void SolveFriction(Collision& collision);
    //void SolveFrictionPair(Collision& collision1, Collision& collision2);

    //void SolveJointPosition(struct Joint& joint);
    //void SolveJointVelocity(Joint& joint);

    //void SolveMouseJoint(struct MouseJoint& mouseJoint, float DeltaTime);

    //void ApplyRestitution(Collision& collision);
};

