#pragma once

#include "ParticleManager.h"


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
	float nearPressureMultiplier = 150;

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
	void addParticle(vec3 localPosition);
	void spawnRandomParticle();

	void update(float deltaTime);
	void tick();

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

	void calculateDensity(int particleIndex);

	void applyPressure(int particleIndex);

	void applyBoundaryConstraint(int particleIndex);
};