#pragma once

#include "BaseAppClasses.h"

#include "Camera.h"
#include "PhysicsEngine.h"
#include "PhysicsBody.h"
#include "Registry.h"

#define MAX_OBJECTS 32

class GameEngine : public App3D {
protected:
	vec3 worldUp;
	Camera camera;

	PhysicsEngine physEng;

public:
	Plane ground;

public:
	GameEngine();
	virtual ~GameEngine() { 
		int objectCount = Registry<GameObject>::count;
		for (int i = 0; i < objectCount; i++) { 
			GameObject* test = Registry<GameObject>::entries[0];
			float a = 5;
			delete test;
		}
	}

	virtual bool startup(int windowWidth, int windowHeight) override;
	virtual bool update() override;
	virtual void draw() override;
	virtual void shutdown() override;

	virtual void onMouseMoved(MouseInfo mouse) override;
};
