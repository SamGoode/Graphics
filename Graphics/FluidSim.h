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
	float solverIterations = 2;

	bool isSimGPU = true;

	// units in metres and kgs
	// particle spherical volume would be pi*4*r^3/3
	// 0.004*pi/3 metres cubed
	// water density is 1000 kg per cube metre
	// with water density, 0.1 m radius sphere would have mass PI*4/3 kgs.
	// approx. 4.18879 kilograms
	// divide this evenly among estimated number of neighbouring particles (30-40) n = 30 for now.

	vec3 gravity = vec3(0);
	float particleRadius = 0.f;
	float smoothingRadius = 0.f; // density kernel radius
	float restDensity = 0.f;
	float particleMass = 0.f;

	// Precomputed values
	float sqrSmoothingRadius = 0.f;
	float normFactor_P6 = 0.f;
	float normFactor_S = 0.f;


	// Mullen.M parameters
	float epsilon = 0.f;
	float k = 0.f;
	int N = 0;
	float densityDeltaQ = 0.f;

	// Clavet.S parameters
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
		float particleMass;

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

	void init(vec3 _position, vec3 _bounds, vec3 _gravity, float _particleRadius = 0.4f,
		float _restDensity = 1000.f, float _stiffness = 20.f, float _nearStiffness = 80.f) {
		
		position = _position;
		bounds = _bounds;

		gravity = _gravity;

		particleRadius = _particleRadius;
		smoothingRadius = particleRadius / 4.f; // (recommended on compsci stack exchange)
		restDensity = _restDensity;
		
		// particle mass calculation based on radius and rest density
		float particleVolume = (particleRadius * particleRadius * particleRadius * 4.f * glm::pi<float>()) / 3.f; // metres^3
		constexpr unsigned int estimatedNeighbours = 20;
		particleMass = (particleVolume * restDensity) / (float)estimatedNeighbours; // kgs

		// Precomputed values
		sqrSmoothingRadius = smoothingRadius * smoothingRadius;
		normFactor_P6 = 315.f / (64.f * glm::pi<float>() * glm::pow<float>(smoothingRadius, 9));
		normFactor_S = 45.f / (glm::pi<float>() * glm::pow<float>(smoothingRadius, 6));


		// Mullen.M parameters
		epsilon = 0.5f;
		k = 0.1f;
		N = 4;
		float deltaQ = 0.3f * smoothingRadius;
		densityDeltaQ = particleMass * polySixKernel(deltaQ, smoothingRadius, normFactor_P6);


		// Clavet.S parameters
		stiffness = _stiffness;
		nearStiffness = _nearStiffness;

		spatialHashGrid.init(smoothingRadius, MAX_PARTICLES, MAX_PARTICLES_PER_CELL);
		
		configUBO.init();
		particleSSBO.init();
		dispatchIndirect.init();

		particleComputeShader.init("particleCompute.glsl");
		computeHashTableShader.init("buildHashTable.glsl");
		computeDensityShader.init("computeDensity.glsl");
		computePressureShader.init("computePressure.glsl");
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
			.particleMass = particleMass,

			.stiffness = stiffness,
			.nearStiffness = nearStiffness,

			.timeStep = fixedTimeStep,
			.particleCount = particleCount
		};

		//std::memcpy(particleSSBO.buffer.hashTable, spatialHashGrid.getHashTable(), MAX_PARTICLES * sizeof(unsigned int));
		//std::memcpy(particleSSBO.buffer.cellEntries, spatialHashGrid.getCellEntries(), MAX_PARTICLES * sizeof(unsigned int));
		//std::memcpy(particleSSBO.buffer.cells, spatialHashGrid.getCells(), spatialHashGrid.getUsedCells() * MAX_PARTICLES_PER_CELL * sizeof(unsigned int));
		unsigned int usedCells = 0;
		unsigned int defaultHash = 0xFFFFFFFF;
		//particleSSBO.clearNamedSubData(GL_R32UI, 15 * MAX_PARTICLES * sizeof(float), sizeof(float), GL_RED_INTEGER, GL_UNSIGNED_INT, &usedCells);
		particleSSBO.clearNamedSubData(GL_R32UI, ((16 * MAX_PARTICLES) + 1) * sizeof(float), MAX_PARTICLES * sizeof(float), GL_RED_INTEGER, GL_UNSIGNED_INT, &defaultHash);
		//particleSSBO.clearNamedSubData(GL_R32UI, ((17 * MAX_PARTICLES) + 1) * sizeof(float), MAX_PARTICLES * sizeof(float), GL_RED_INTEGER, GL_UNSIGNED_INT, &usedCells);


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
	// Muller.M kernels
	static float polySixKernel(float dist, float radius, float normFactor) {
		float value = radius * radius - dist * dist;
		return value * value * value * normFactor;
	}

	static float polySixKernelSqr(float sqrDist, float sqrRadius, float normFactor) {
		float value = sqrRadius - sqrDist;
		return value * value * value * normFactor;
	}

	// just for show, this never gets used
	static float spikyKernel(float dist, float radius) {
		constexpr float a = 15 / glm::pi<float>();
		float normalizationFactor = a * (float)glm::pow(radius, -6);

		float value = radius - dist;
		return value * value * value * normalizationFactor;
	}

	static float spikyKernelGradient(float dist, float radius, float normFactor) {
		float value = radius - dist;
		return value * value * normFactor;
	}


	// Clavet.S kernels
	static float densityKernel(float dist, float radius) {
		float value = 1 - (dist / radius);
		return value * value;
	}

	static float nearDensityKernel(float dist, float radius) {
		float value = 1 - (dist / radius);
		return value * value * value;
	}

	static float calculatePressure(float density, float restDensity, float stiffness) {
		return (density - restDensity) * stiffness;
	}

	static float calculatePressureForce(float dist, float radius, float pressure, float nearPressure) {
		float weight = 1 - (dist / radius);
		return pressure * weight + nearPressure * weight * weight * 0.5f;
	}


	// Muller paper on enforcing incompressibility as a position constraint
	void calculateLambda(unsigned int particleIndex);
	void calculateDisplacement(unsigned int particleIndex);

	// Clavet paper with double density kernel
	void calculateDensity(unsigned int particleIndex);
	void applyPressure(unsigned int particleIndex);

	void applyBoundaryConstraints(unsigned int particleIndex);
	void applyBoundaryPressure(unsigned int particleIndex);
};