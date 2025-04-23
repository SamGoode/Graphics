#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>

#include "SpatialGrid.h"
#include "UniformBuffer.h"
#include "ShaderStorageBuffer.h"
#include "Shader.h"


using glm::uvec2;
using glm::uvec3;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::uvec4;

#define MAX_PARTICLES 4096
#define MAX_PARTICLES_PER_CELL 16


// SPH Fluid simulation within a bounding box
class FluidSimSPH {
private:
	vec3 position;
	vec3 bounds;

	const int maxTicksPerUpdate = 5;
	const float fixedTimeStep = 0.01f;
	float accumulatedTime = 0.f;

	vec3 gravity;
	float smoothingRadius; // density kernel radius
	float restDensity;
	float stiffness;
	float nearStiffness;

	unsigned int particleCount = 0;
	vec3 positions[MAX_PARTICLES];
	vec3 previousPositions[MAX_PARTICLES];
	vec3 velocities[MAX_PARTICLES];
	float densities[MAX_PARTICLES];
	float nearDensities[MAX_PARTICLES];

	struct uboData {
		vec4 boundsMin;
		vec4 boundsMax;

		vec4 gravity;
		float smoothingRadius;
		float restDensity;
		float stiffness;
		float nearStiffness;

		float timeStep;
		unsigned int particleCount;
	};

	struct ssboData {
		//unsigned int particleCount;
		//float smoothingRadius; // All particles share same radius
		//vec2 padding;
		vec4 positions[MAX_PARTICLES];
		vec4 previousPositions[MAX_PARTICLES];
		vec4 velocities[MAX_PARTICLES];
		vec4 pressureDisplacements[MAX_PARTICLES];
		float densities[MAX_PARTICLES];
		float nearDensities[MAX_PARTICLES];

		unsigned int usedCells;
		unsigned int hashTable[MAX_PARTICLES];
		unsigned int cellEntries[MAX_PARTICLES];
		unsigned int cells[MAX_PARTICLES * MAX_PARTICLES_PER_CELL];
	};

	SpatialHashGrid spatialHashGrid;

	UniformBuffer<uboData> configUBO;
	ShaderStorageBuffer<ssboData> particleSSBO;

	ComputeShader particleComputeShader;
	ComputeShader particleComputeShader2;

public:
	FluidSimSPH() {}
	~FluidSimSPH() {}

	void init(vec3 _position, vec3 _bounds, vec3 _gravity, float _smoothingRadius = 0.1f,
		float _restDensity = 2.f, float _stiffness = 10.f, float _nearStiffness = 40.f) {
		
		position = _position;
		bounds = _bounds;

		gravity = _gravity;
		smoothingRadius = _smoothingRadius;
		restDensity = _restDensity;
		stiffness = _stiffness;
		nearStiffness = _nearStiffness;

		spatialHashGrid.init(smoothingRadius, MAX_PARTICLES, MAX_PARTICLES_PER_CELL);
		
		configUBO.init();
		particleSSBO.init();

		particleComputeShader.init("particleCompute.glsl");
		particleComputeShader2.init("particleCompute2.glsl");

		for (int i = 0; i < MAX_PARTICLES; i++) {
			particleSSBO.buffer.hashTable[i] = 0xFFFFFFFF;
			particleSSBO.buffer.cellEntries[i] = 0;
		}

	}

	unsigned int getParticleCount() { return particleCount; }
	void clearParticles() { particleCount = 0; }
	void addParticle(vec3 localPosition);
	void spawnRandomParticles(unsigned int spawnCount = 1);

	void update(float deltaTime);
	void tick();

	void sendDataToGPU() {
		configUBO.buffer = {
			.boundsMin = vec4(position, 1),
			.boundsMax = vec4(position + bounds, 1),

			.gravity = vec4(gravity, 1),
			.smoothingRadius = smoothingRadius,
			.restDensity = restDensity,
			.stiffness = stiffness,
			.nearStiffness = nearStiffness,

			.timeStep = fixedTimeStep,
			.particleCount = particleCount
		};

		for (int i = 0; i < particleCount; i++) {
			particleSSBO.buffer.positions[i] = vec4(positions[i], 1);
			particleSSBO.buffer.velocities[i] = vec4(0, 0, 0, 1);
		}

		//std::memcpy(particleSSBO.buffer.hashTable, spatialHashGrid.getHashTable(), MAX_PARTICLES * sizeof(unsigned int));
		//std::memcpy(particleSSBO.buffer.cellEntries, spatialHashGrid.getCellEntries(), MAX_PARTICLES * sizeof(unsigned int));
		//std::memcpy(particleSSBO.buffer.cells, spatialHashGrid.getCells(), spatialHashGrid.getUsedCells() * MAX_PARTICLES_PER_CELL * sizeof(unsigned int));

		configUBO.subData();
		particleSSBO.subData();
	}

	void bindConfigUBO(GLuint bindingIndex) {
		configUBO.bind(bindingIndex);
	}

	void bindParticleSSBO(GLuint bindingIndex) {
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

	void calculateDensity(unsigned int particleIndex);
	void applyPressure(unsigned int particleIndex);

	void applyBoundaryConstraints(unsigned int particleIndex);
	void applyBoundaryPressure(unsigned int particleIndex);
};