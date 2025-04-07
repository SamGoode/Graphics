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


#define MAX_PARTICLES 128

class ParticleManager {
private:
	unsigned int particleSSBO;

	struct ssbo {
		unsigned int count = 0;
		float radius = 1.f; // All particles share same radius
		vec2 padding;
		vec4 positions[MAX_PARTICLES];
	} buffer;


public:
	ParticleManager() {}
	~ParticleManager() {
		glDeleteBuffers(1, &particleSSBO);
	}

	void init() {
		glGenBuffers(1, &particleSSBO);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSSBO);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(ssbo), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void subData() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, particleSSBO);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(ssbo), &buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void bind(GLuint bindingIndex) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, particleSSBO);
	}

	unsigned int getCount() { return buffer.count; }
	void setParticleRadius(float radius) { buffer.radius = radius; }

	void clearParticles() { buffer.count = 0; }

	void addParticle(vec3 position) {
		assert(buffer.count < MAX_PARTICLES && "Max particle limit reached");

		buffer.positions[buffer.count++] = vec4(position, 1);
	}
};