#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>

#include "common.h"

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


// SPH Fluid simulation within a bounding box
class FluidSimSPH {
private:
	vec3 position = vec3(0);
	vec3 bounds = vec3(0);

	const unsigned int maxTicksPerUpdate = 8;
	const float fixedTimeStep = 0.01f;
	float accumulatedTime = 0.f;

	bool isSimGPU = false;

	vec3 gravity = vec3(0);
	float smoothingRadius = 0.f; // density kernel radius
	float restDensity = 0.f;
	float stiffness = 0.f;
	float nearStiffness = 0.f;

	unsigned int particleCount = 0;
	//vec3 positions[MAX_PARTICLES];
	//vec3 previousPositions[MAX_PARTICLES];
	//vec3 velocities[MAX_PARTICLES];

	float lambdas[MAX_PARTICLES];
	vec3 displacements[MAX_PARTICLES];
	
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
		vec4 positions[MAX_PARTICLES];
		vec4 previousPositions[MAX_PARTICLES];
		vec4 velocities[MAX_PARTICLES];

		float lambdas[MAX_PARTICLES];
		float densities[MAX_PARTICLES];
		float nearDensities[MAX_PARTICLES];

		unsigned int usedCells;
		unsigned int hashes[MAX_PARTICLES];
		unsigned int hashTable[MAX_PARTICLES];
		unsigned int cellEntries[MAX_PARTICLES];
		unsigned int cells[MAX_PARTICLES * MAX_PARTICLES_PER_CELL];
	};

	SpatialHashGrid spatialHashGrid;

	UniformBuffer<uboData> configUBO;
	ShaderStorageBuffer<ssboData> particleSSBO;
	DispatchIndirectBuffer dispatchIndirect;

	vec4* positions = particleSSBO.buffer.positions;
	vec4* previousPositions = particleSSBO.buffer.previousPositions;
	vec4* velocities = particleSSBO.buffer.velocities;


	ComputeShader particleComputeShader;
	ComputeShader computeHashTableShader;
	ComputeShader computeDensityShader;
	ComputeShader computePressureShader;


public:
	FluidSimSPH() {}
	~FluidSimSPH() {}

	void init(vec3 _position, vec3 _bounds, vec3 _gravity, float _smoothingRadius = 0.15f,
		float _restDensity = 2.f, float _stiffness = 20.f, float _nearStiffness = 80.f) {
		
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
		dispatchIndirect.init();

		particleComputeShader.init("shaders/particleCompute.glsl");
		computeHashTableShader.init("shaders/buildHashTable.glsl");
		computeDensityShader.init("shaders/computeDensity.glsl");
		computePressureShader.init("shaders/computePressure.glsl");
	}

	bool isRunningOnGPU() { return isSimGPU; }
	bool* shouldRunOnGPU() { return &isSimGPU; }

	unsigned int getParticleCount() { return particleCount; }
	void clearParticles() { particleCount = 0; }
	void addParticle(vec3 localPosition);
	void spawnRandomParticles(unsigned int spawnCount = 1);

	void updateSpatialGrid() { spatialHashGrid.generateHashTable(particleCount, positions); }
	void resetSpatialGrid() { spatialHashGrid.resetHashTable(); }

	void update(float deltaTime);
	void tickSimGPU();
	void tickSimCPU();

	void sendDataToGPU() {
		// This whole current SSBO template buffer system is scuffed
		// temporary for now, will change later

		configUBO.buffer = {
			.boundsMin = vec4(position, 0),
			.boundsMax = vec4(position + bounds, 0),

			.gravity = vec4(gravity, 0),
			.smoothingRadius = smoothingRadius,
			.restDensity = restDensity,
			.stiffness = stiffness,
			.nearStiffness = nearStiffness,

			.timeStep = fixedTimeStep,
			.particleCount = particleCount
		};

		std::memcpy(particleSSBO.buffer.hashTable, spatialHashGrid.getHashTable(), MAX_PARTICLES * sizeof(unsigned int));
		std::memcpy(particleSSBO.buffer.cellEntries, spatialHashGrid.getCellEntries(), MAX_PARTICLES * sizeof(unsigned int));
		std::memcpy(particleSSBO.buffer.cells, spatialHashGrid.getCells(), spatialHashGrid.getUsedCells() * MAX_PARTICLES_PER_CELL * sizeof(unsigned int));

		configUBO.subData();
		particleSSBO.subData();
	}

	void pullDataFromGPU() {
		particleSSBO.subData(0, particleCount * sizeof(vec4), particleSSBO.buffer.positions);
		particleSSBO.subData(MAX_PARTICLES * sizeof(vec4), particleCount * sizeof(vec4), particleSSBO.buffer.previousPositions);
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

	// just for show, don't actually compute the kernel like this
	static float polySixKernel(float radius, float dist) {
		constexpr float a = 315/(glm::pi<float>() * 64);
		float normalizationFactor = a * (float)glm::pow(radius, -9);
		
		float value = radius * radius - dist * dist;
		return value * value * value * normalizationFactor;
	}

	// just for show, don't actually compute the kernel like this
	static float spikyKernel(float radius, float dist) {
		constexpr float a = 15 / glm::pi<float>();
		float normalizationFactor = a * (float)glm::pow(radius, -6);

		float value = radius - dist;
		return value * value * value * normalizationFactor;
	}

	// just for show, don't actually compute the kernel like this
	static float spikyKernelGradient(float radius, float dist) {
		constexpr float a = 45 / glm::pi<float>();
		float normalizationFactor = a * (float)glm::pow(radius, -6);

		float value = radius - dist;
		return value * value * normalizationFactor;
	}

	// Clavet paper with double density kernel
	void calculateDensity(unsigned int particleIndex);
	void applyPressure(unsigned int particleIndex);

	// Muller paper on enforcing incompressibility as a position constraint
	void calculateLambda(unsigned int particleIndex);
	void calculateDisplacement(unsigned int particleIndex);

	void applyBoundaryConstraints(unsigned int particleIndex);
	void applyBoundaryPressure(unsigned int particleIndex);
};