#pragma once

#include "BaseAppClasses.h"

#include "Camera.h"
#include "PhysicsEngine.h"
#include "PhysicsObject.h"
#include "Registry.h"

#include "Mesh.h"
#include "Texture.h"

#include "Framebuffer.h"


class GameEngine : public App3D {
protected:
	vec3 worldUp;
	Camera camera;

	vec3 ambientLighting;
	vec3 lightColor;
	vec3 lightDirection;

	PhysicsEngine physEng;

	Mesh meshes[3];
	Texture textures[3];

	FrameBuffer gpassFBO;
	FrameBuffer lightFBO;

	Shader gpassShader;
	Shader lightShader;
	Shader compositeShader;


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
