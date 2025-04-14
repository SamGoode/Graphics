#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>

#include "SpatialGrid.h"
#include "ShaderStorageBuffer.h"


using glm::uvec2;
using glm::uvec3;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::uvec4;


#define MAX_PARTICLES 1024

// SPH Fluid simulation within a bounding box
class FluidSimSPH {
private:
	vec3 position;
	vec3 bounds;

	const int maxTicksPerUpdate = 5;
	const float fixedTimeStep = 0.01f;
	float accumulatedTime = 0.f;

	vec3 gravity;
	float particleRadius;
	float smoothingRadius; // radius of influence
	float restDensity;
	float stiffness;
	float nearStiffness;

	unsigned int particleCount = 0;
	vec3 positions[MAX_PARTICLES];
	vec3 projectedPositions[MAX_PARTICLES];
	vec3 velocities[MAX_PARTICLES];
	float densities[MAX_PARTICLES];
	float nearDensities[MAX_PARTICLES];

	struct data {
		vec4 simPosition;
		vec4 simBounds;
		glm::ivec4 gridBounds;
		unsigned int particleCount;
		float particleRadius; // All particles share same radius
		float cellSize;
		float padding;
		vec4 positions[MAX_PARTICLES];
		ivec2 hashList[MAX_PARTICLES];
		ivec2 lookupTable[8192]; // cell count
	};
	ShaderStorageBuffer<data> particleSSBO;
	SpatialGrid spatialGrid;


public:
	FluidSimSPH() {}
	~FluidSimSPH() {}

	void init(vec3 _position, vec3 _bounds, vec3 _gravity, float _particleRadius = 0.2f,
		float _restDensity = 1.5f, float _stiffness = 10, float _nearStiffness = 40) {
		
		position = _position;
		bounds = _bounds;

		gravity = _gravity;
		particleRadius = _particleRadius;
		smoothingRadius = particleRadius * 2.f;
		restDensity = _restDensity;
		stiffness = _stiffness;
		nearStiffness = _nearStiffness;

		particleSSBO.init();

		for (int i = 0; i < 8192; i++) {
			particleSSBO.buffer.lookupTable[i] = uvec2(-1, -1);
		}

		spatialGrid.init(bounds, smoothingRadius, MAX_PARTICLES);
	}

	unsigned int getParticleCount() { return particleCount; }
	void clearParticles() { particleCount = 0; }
	void addParticle(vec3 localPosition);
	void spawnRandomParticle();

	void update(float deltaTime);
	void tick();

	void sendDataToGPU(const glm::mat4& view) {
		particleSSBO.buffer = {
			.simPosition = vec4(position, 1),
			.simBounds = vec4(bounds, 0),
			.gridBounds = glm::ivec4(spatialGrid.getGridBounds(), 0),
			.particleCount = particleCount,
			.particleRadius = particleRadius,
			.cellSize = smoothingRadius
		};

		// build spatial grid with current positions (not projected)
		//spatialGrid.buildSpatialGrid(particleCount, positions);

		for (int i = 0; i < particleCount; i++) {
			particleSSBO.buffer.positions[i] = vec4(projectedPositions[i], 1);
			particleSSBO.buffer.hashList[i] = spatialGrid.getHashList()[i];
		}


		for (int i = 0; i < spatialGrid.getCellCount(); i++) {
			particleSSBO.buffer.lookupTable[i] = spatialGrid.getLookupTable()[i];
		}

		particleSSBO.subData();
	}

	void bindSSBO(GLuint bindingIndex) {
		particleSSBO.bind(bindingIndex);
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

	static float calculatePressure(float density, float restDensity, float stiffness) {
		return (density - restDensity) * stiffness;
	}

	static float calculatePressureForce(float pressure, float nearPressure, float radius, float dist) {
		float weight = 1 - (dist / radius);
		return pressure * weight + nearPressure * weight * weight * 0.5f;
	}

	void calculateDensity(int particleIndex);
	void applyPressure(int particleIndex);
	void applyBoundaryPressure(int particleIndex);
};