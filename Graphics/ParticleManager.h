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
protected:
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


// SPH Fluid simulation within a bounding box
class FluidSimSPH {
private:
	vec3 position;
	vec3 bounds;

	const int maxTicksPerUpdate = 5;
	const float fixedTimeStep = 0.01f;
	float accumulatedTime = 0.f;

	vec3 gravity = vec3(0, 0, -4.f);
	float particleRadius;
	float smoothingRadius;
	float targetDensity = 2.5f;
	float pressureMultiplier = 100;
	float nearPressureMultiplier = 100;

	unsigned int particleCount = 0;
	vec3 positions[MAX_PARTICLES];
	vec3 projectedPositions[MAX_PARTICLES];
	vec3 velocities[MAX_PARTICLES];
	float densities[MAX_PARTICLES];
	float nearDensities[MAX_PARTICLES];


public:
	FluidSimSPH() {}
	~FluidSimSPH() {}

	void init(vec3 _position, vec3 _bounds, float _particleRadius) {
		position = _position;
		bounds = _bounds;

		particleRadius = _particleRadius;
		smoothingRadius = _particleRadius * 2;
	}

	void clearParticles() { particleCount = 0; }

	void addParticle(vec3 localPosition) {
		assert(particleCount < MAX_PARTICLES);

		positions[particleCount] = localPosition;
		projectedPositions[particleCount] = localPosition;
		particleCount++;
	}

	// Spawns random particle within bounding box
	void spawnRandomParticle() {
		assert(particleCount < MAX_PARTICLES);

		vec3 randomPosition = glm::linearRand(vec3(0), bounds);

		positions[particleCount] = randomPosition;
		projectedPositions[particleCount] = randomPosition;
		particleCount++;
	}

	void update(float deltaTime) {
		accumulatedTime += deltaTime;

		for (int step = 0; step < maxTicksPerUpdate && accumulatedTime > fixedTimeStep; step++) {
			stepForward();
			accumulatedTime -= fixedTimeStep;
		}
	}

	void stepForward() {
		// Apply gravity
		for (int i = 0; i < particleCount; i++) {
			velocities[i] += gravity * fixedTimeStep;
		}

		// Project particle positions
		for (int i = 0; i < particleCount; i++) {
			positions[i] = projectedPositions[i];
			projectedPositions[i] += velocities[i] * fixedTimeStep;
		}

		// Boundaries
		for (int i = 0; i < particleCount; i++) {
			applyBoundaryConstraint(i);
		}

		// Compute implicit velocity
		for (int i = 0; i < particleCount; i++) {
			velocities[i] = (projectedPositions[i] - positions[i]) / fixedTimeStep;
		}
	}

	void transferData(ParticleManager& particleManager) {
		particleManager.setParticleRadius(particleRadius);

		particleManager.clearParticles();
		for (int i = 0; i < particleCount; i++) {
			particleManager.addParticle(positions[i] + position);
		}
	}

private:
	static float densityKernel(float radius, float dist) {
		float value = 1 - (dist / radius);
		return value * value;
	}

	static float nearDensityKernel(float radius, float dist) {
		float value = 1 - (dist / radius);
		return value * value * value;
	}

	void applyBoundaryConstraint(int particleIndex) {
		float pressure = targetDensity * nearPressureMultiplier;

		vec3& particlePos = projectedPositions[particleIndex];
		// X-Axis
		if (particlePos.x + smoothingRadius > bounds.x) {
			float dist = bounds.x - particlePos.x;
			float value = 1 - (dist / smoothingRadius);

			particlePos.x -= pressure * value * value * fixedTimeStep * fixedTimeStep;
		}
		else if (particlePos.x - smoothingRadius < 0) {
			float dist = particlePos.x;
			float value = 1 - (dist / smoothingRadius);

			particlePos.x += pressure * value * value * fixedTimeStep * fixedTimeStep;
		}
		// Y-Axis
		if (particlePos.y + smoothingRadius > bounds.y) {
			float dist = bounds.y - particlePos.y;
			float value = 1 - (dist / smoothingRadius);

			particlePos.y -= pressure * value * value * fixedTimeStep * fixedTimeStep;
		}
		else if (particlePos.y - smoothingRadius < 0) {
			float dist = particlePos.y;
			float value = 1 - (dist / smoothingRadius);

			particlePos.y += pressure * value * value * fixedTimeStep * fixedTimeStep;
		}
		// Z-Axis
		if (particlePos.z + smoothingRadius > bounds.z) {
			float dist = bounds.z - particlePos.z;
			float value = 1 - (dist / smoothingRadius);

			particlePos.z -= pressure * value * value * fixedTimeStep * fixedTimeStep;
		}
		else if (particlePos.z - smoothingRadius < 0) {
			float dist = particlePos.z;
			float value = 1 - (dist / smoothingRadius);

			particlePos.z += pressure * value * value * fixedTimeStep * fixedTimeStep;
		}
	}
};