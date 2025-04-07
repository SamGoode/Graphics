#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>


using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;


#define MAX_PARTICLES 100

class ParticleManager {
private:
	struct particle {
		vec3 position;
		float padding;
	};

	// All particles share same radius
	//struct ubo {
	//	unsigned int count;
	//	float radius;
	//	vec2 padding;
	//	particle particles[MAX_PARTICLES];
	//} buffer;

	struct ssbo {
		unsigned int count;
		float radius;
		vec2 padding;
		particle particles[MAX_PARTICLES];
	} buffer;

	//unsigned int particleUBO;
	unsigned int particleSSBO;

	unsigned int particleCount = 0;
	float particleRadius = 2.f;
	particle particles[MAX_PARTICLES];


public:
	ParticleManager() {}
	~ParticleManager() {
		//glDeleteBuffers(1, &particleUBO);
		glDeleteBuffers(1, &particleSSBO);
	}

	void init() {
		//glGenBuffers(1, &particleUBO);
		//glBindBuffer(GL_UNIFORM_BUFFER, particleUBO);
		//glBufferData(GL_UNIFORM_BUFFER, sizeof(ubo), &buffer, GL_STATIC_DRAW);
		//glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glGenBuffers(1, &particleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssbo), &buffer, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void prepRender(const mat4& viewMatrix) {
		// temporary
		// for now we'll convert particle positions to view space here

		buffer.count = particleCount;
		buffer.radius = particleRadius;

		for (int i = 0; i < particleCount; i++) {
			vec3 vPosition = viewMatrix * vec4(particles[i].position, 1);

			buffer.particles[i].position = vPosition;
		}

		//glBindBuffer(GL_UNIFORM_BUFFER, particleUBO);
		//glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(ubo), &buffer);
		//glBindBuffer(GL_UNIFORM_BUFFER, 0);

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ssbo), &buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void bind(GLuint bindingIndex) {
		//glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, particleUBO);

		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, particleSSBO);
	}

	void clearParticles() {
		particleCount = 0;
	}

	void addParticle(vec3 position) {
		assert(particleCount < MAX_PARTICLES && "Max particle limit reached");

		particles[particleCount++] = { position };
	}
};