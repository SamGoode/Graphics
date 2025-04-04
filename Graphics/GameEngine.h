#pragma once

#include "BaseAppClasses.h"

#include "Camera.h"
#include "PhysicsEngine.h"

#include "Mesh.h"
#include "Texture.h"
#include "PointLight.h"

#include "Framebuffer.h"

#include "ECS.h"


class GameEngine : public App3D {
protected:
	vec3 worldUp;
	Camera camera;

	ECS::ECSManager ecs;

	ECS::uint vanEntity;
	float timePassed = 0.f;

	vec3 ambientLighting;
	vec3 lightColor;
	vec3 lightDirection;

	PhysicsEngine physicsEngine;

	const int meshCount = 5;
	Mesh meshes[5];
	const int textureCount = 4;
	Texture textures[4];
	PointLight pointLights;

	FrameBuffer shadowFBO;
	FrameBuffer gpassFBO;
	FrameBuffer lightFBO;
	FrameBuffer raymarchFBO;

	Shader shadowShader;
	Shader gpassShader;
	Shader lightShader;
	Shader pointLightShader;
	Shader compositeShader;
	Shader raymarchShader;


public:
	GameEngine();
	virtual ~GameEngine() = default;

	virtual bool init(int windowWidth, int windowHeight) override;
	virtual bool update() override;
	virtual void render() override;
	virtual void shutdown() override;

	virtual void onMouseMoved(MouseInfo mouse) override;
};
