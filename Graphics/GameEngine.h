#pragma once

#include "BaseAppClasses.h"

#include "Camera.h"
#include "PhysicsEngine.h"

#include "Mesh.h"
#include "Texture.h"
#include "PointLight.h"
#include "ParticleManager.h"

#include "FrameBuffer.h"

#include "ECS.h"


// data layout must follow std140 format
template<typename DataLayout>
class UniformBuffer {
private:
	unsigned int ubo;

public:
	DataLayout buffer;

public:
	UniformBuffer() {}
	~UniformBuffer() {
		glDeleteBuffers(1, &ubo);
	}

	void init() {
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(DataLayout), &buffer, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void subData() {
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DataLayout), &buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void bind(GLuint bindingIndex) {
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, ubo);
	}
};

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

	ParticleManager particleManager;

	struct pvmLayout {
		mat4 view;
		mat4 projection;
		mat4 viewInverse;
		mat4 projectionInverse;
	};
	UniformBuffer<pvmLayout> pvmUBO;

	FrameBuffer shadowFBO;
	FrameBuffer gpassFBO;
	FrameBuffer lightFBO;

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
