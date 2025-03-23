#pragma once

#include "BaseAppClasses.h"

#include "Camera.h"
#include "PhysicsEngine.h"
#include "PhysicsBody.h"
#include "Registry.h"





class GameEngine : public App3D {
protected:
	vec3 worldUp;
	Camera camera;

	PhysicsEngine physEng;

	Mesh bunny;

public:
	GameEngine();
	virtual ~GameEngine() { 
		int objectCount = Registry<GameObject>::count;
		for (int i = 0; i < objectCount; i++) { 
			delete Registry<GameObject>::entries[0];
		}
	}

	virtual bool startup(int windowWidth, int windowHeight) override;
	virtual bool update() override;
	virtual void draw() override;
	virtual void shutdown() override;

	virtual void onMouseMoved(MouseInfo mouse) override;
};
