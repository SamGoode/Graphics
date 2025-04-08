#pragma once

#include "ParticleManager.h"
#include "SpatialGrid.h"

using glm::uvec2;
using glm::uvec3;



// SPH Fluid simulation within a bounding box
class FluidSimSPH {
private:
	vec3 position;
	vec3 bounds;

	SpatialGrid spatialGrid;

	struct data {
		unsigned int particleCount;
		float particleRadius; // All particles share same radius
		vec2 padding;
		vec4 positions[MAX_PARTICLES];
	};
	ShaderStorageBuffer<data> particleSSBO;

	const int maxTicksPerUpdate = 5;
	const float fixedTimeStep = 0.01f;
	float accumulatedTime = 0.f;

	vec3 gravity;
	float particleRadius;
	float smoothingRadius; // radius of influence
	float restDensity;
	float pressureMultiplier;
	float nearPressureMultiplier;

	unsigned int particleCount = 0;
	vec3 positions[MAX_PARTICLES];
	vec3 projectedPositions[MAX_PARTICLES];
	vec3 velocities[MAX_PARTICLES];
	float densities[MAX_PARTICLES];
	float nearDensities[MAX_PARTICLES];


public:
	FluidSimSPH() {}
	~FluidSimSPH() {}

	void init(vec3 _position, vec3 _bounds, vec3 _gravity, float _particleRadius = 0.25f,
		float _restDensity = 1.5f, float _pressureMultiplier = 50, float _nearPressureMultiplier = 200) {
		
		float _smoothingRadius = _particleRadius * 2.f;

		spatialGrid.init(_bounds, _smoothingRadius, MAX_PARTICLES);

		particleSSBO.init();

		position = _position;
		bounds = _bounds;

		gravity = _gravity;
		particleRadius = _particleRadius;
		smoothingRadius = _smoothingRadius;
		restDensity = _restDensity;
		pressureMultiplier = _pressureMultiplier;
		nearPressureMultiplier = _nearPressureMultiplier;
	}

	unsigned int getParticleCount() { return particleCount; }
	void clearParticles() { particleCount = 0; }
	void addParticle(vec3 localPosition);
	void spawnRandomParticle();

	void update(float deltaTime);
	void tick();

	void sendDataToGPU() {
		particleSSBO.buffer = {
			.particleCount = particleCount,
			.particleRadius = particleRadius,
		};

		for (int i = 0; i < particleCount; i++) {
			particleSSBO.buffer.positions[i] = vec4(positions[i] + position, 1);
		}

		particleSSBO.subData();
	}

	void bindSSBO(GLuint bindingIndex) {
		particleSSBO.bind(bindingIndex);
	}

	//void transferData(ParticleManager& particleManager) {
	//	particleManager.setParticleRadius(particleRadius);

	//	particleManager.clearParticles();
	//	for (int i = 0; i < particleCount; i++) {
	//		particleManager.addParticle(positions[i] + position);
	//	}
	//}

private:
	static float densityKernel(float radius, float dist) {
		float value = 1 - (dist / radius);
		return value * value;
	}

	static float nearDensityKernel(float radius, float dist) {
		float value = 1 - (dist / radius);
		return value * value * value;
	}

	void calculateDensity(int particleIndex);
	void applyPressure(int particleIndex);
	void applyBoundaryPressure(int particleIndex);
};