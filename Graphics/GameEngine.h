#pragma once

#include "BaseAppClasses.h"

#include "Camera.h"
#include "PhysicsEngine.h"

#include "Mesh.h"
#include "Texture.h"
#include "PointLight.h"
#include "FluidSim.h"

#include "UniformBuffer.h"
#include "FrameBuffer.h"

#include "ECS.h"


class GameEngine : public App3D {
protected:
	vec3 worldUp;
	Camera camera;

	ECS::ECSManager ecs;

	ECS::uint vanEntity;

	vec3 ambientLighting;
	vec3 lightColor;
	vec3 lightDirection;

	PhysicsEngine physicsEngine;

	const int meshCount = 5;
	Mesh meshes[5];
	const int textureCount = 4;
	Texture textures[4];
	PointLight pointLights;

	FluidSimSPH fluidSim;

	struct pvmLayout {
		mat4 view;
		mat4 projection;
		mat4 viewInverse;
		mat4 projectionInverse;
		vec4 cameraPos;
	};
	UniformBuffer<pvmLayout> pvmUBO;

	FrameBuffer shadowFBO;
	FrameBuffer fluidDepthFBO;
	FrameBuffer gpassFBO;
	FrameBuffer lightFBO;

	Shader shadowShader;
	Shader gpassShader;
	Shader lightShader;
	Shader pointLightShader;
	Shader compositeShader;
	Shader fluidDepthShader;
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
