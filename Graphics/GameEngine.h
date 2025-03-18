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

	void drawObject(RenderObject* object) {
		switch (object->getID()) {
		case 0:
			gl->addCuboid(object->pos, static_cast<Box*>(object->shape)->extents, object->rot, object->color);
			break;
		case 1:
			gl->addSphere(object->pos, static_cast<Sphere*>(object->shape)->radius, object->color);
			break;
		}
	}

	virtual void onMouseMoved(MouseInfo mouse) override;
};
