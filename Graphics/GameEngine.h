#pragma once

#include "BaseAppClasses.h"

#include "Camera.h"
#include "PhysicsEngine.h"
#include "PhysicsBody.h"


#define MAX_BODIES 32

class GameEngine : public App3D {
protected:
	vec3 worldUp;
	Camera camera;

	PhysicsEngine physEng;

public:
	int bodyCount = 0;
	const int maxBodies = MAX_BODIES;
	GameObject* bodies[MAX_BODIES];

	Plane ground;

public:
	GameEngine();
	virtual ~GameEngine() { for (int i = 0; i < bodyCount; i++) { delete bodies[i]; } }

	virtual bool startup(int windowWidth, int windowHeight) override;
	virtual bool update() override;
	virtual void draw() override;
	virtual void shutdown() override;

	virtual void onMouseMoved(MouseInfo mouse) override;

	GameObject* addGameObject(GameObject* gameObject);
};
