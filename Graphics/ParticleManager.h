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
	struct data {
		unsigned int count;
		float radius;
		vec2 padding;
		particle particles[MAX_PARTICLES];
	} buffer;

	unsigned int particleUBO;

	unsigned int particleCount = 0;
	float particleRadius = 2.f;
	particle particles[MAX_PARTICLES];


public:
	ParticleManager() {}
	~ParticleManager() {
		glDeleteBuffers(1, &particleUBO);
	}

	void init() {
		glGenBuffers(1, &particleUBO);
		glBindBuffer(GL_UNIFORM_BUFFER, particleUBO);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(data), &buffer, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
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

		glBindBuffer(GL_UNIFORM_BUFFER, particleUBO);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(data), &buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void bindUBO(GLuint bindingIndex) {
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, particleUBO);
	}

	void clearParticles() {
		particleCount = 0;
	}

	void addParticle(vec3 position) {
		assert(particleCount < MAX_PARTICLES && "Max particle limit reached");

		particles[particleCount++] = { position };
	}
};