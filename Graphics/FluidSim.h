#pragma once

#include "ModularFluids.h"

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm/glm.hpp>
#include <glm/glm/ext.hpp>
#include <glm/glm/fwd.hpp>

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
	const unsigned int maxTicksPerUpdate = 8;
	const float fixedTimeStep = 0.01f;
	float accumulatedTime = 0.f;
	const unsigned int solverIterations = 2;

	//DispatchIndirectBuffer dispatchIndirect;

	ComputeShader particleComputeShader;
	ComputeShader computeHashTableShader;
	ComputeShader computeDensityShader;
	ComputeShader computePressureShader;

	ISPH_Compute* sim;

public:
	FluidSimSPH() {}
	~FluidSimSPH() { ModularFluids::Destroy(sim); }

	void init(vec3 _position, vec3 _bounds, vec3 _gravity, float _particleRadius = 0.4f,
		float _restDensity = 1000.f, float _stiffness = 20.f, float _nearStiffness = 80.f) {
		
		sim = ModularFluids::Create();
		sim->init(_position, _bounds, _gravity, _particleRadius, _restDensity, _stiffness, _nearStiffness);
		//ModularFluids::Init(sim, _position, _bounds, _gravity, _particleRadius, _restDensity, _stiffness, _nearStiffness);

		//dispatchIndirect.init();

		particleComputeShader.init("shaders/particleCompute.glsl");
		computeHashTableShader.init("shaders/buildHashTable.glsl");
		computeDensityShader.init("shaders/computeDensity.glsl");
		computePressureShader.init("shaders/computePressure.glsl");
	}

	unsigned int getParticleCount() { return sim->getParticleCount(); }
	void spawnRandomParticles(unsigned int spawnCount);
	void clearParticles() { sim->clearParticles(); }

	void update(float deltaTime);
	void tickSimGPU();

	void bindConfigUBO(GLuint bindingIndex) { sim->bindConfigUBO(bindingIndex); }
	void bindParticleSSBO(GLuint bindingIndex) { sim->bindParticleSSBO(bindingIndex); }


private:
	//// Muller.M kernels
	//static float polySixKernel(float dist, float radius, float normFactor) {
	//	float value = radius * radius - dist * dist;
	//	return value * value * value * normFactor;
	//}

	//static float polySixKernelSqr(float sqrDist, float sqrRadius, float normFactor) {
	//	float value = sqrRadius - sqrDist;
	//	return value * value * value * normFactor;
	//}

	//// just for show, this never gets used
	//static float spikyKernel(float dist, float radius) {
	//	constexpr float a = 15 / glm::pi<float>();
	//	float normalizationFactor = a * (float)glm::pow(radius, -6);

	//	float value = radius - dist;
	//	return value * value * value * normalizationFactor;
	//}

	//static float spikyKernelGradient(float dist, float radius, float normFactor) {
	//	float value = radius - dist;
	//	return value * value * normFactor;
	//}


	//// Clavet.S kernels
	//static float densityKernel(float dist, float radius) {
	//	float value = 1 - (dist / radius);
	//	return value * value;
	//}

	//static float nearDensityKernel(float dist, float radius) {
	//	float value = 1 - (dist / radius);
	//	return value * value * value;
	//}

	//static float calculatePressure(float density, float restDensity, float stiffness) {
	//	return (density - restDensity) * stiffness;
	//}

	//static float calculatePressureForce(float dist, float radius, float pressure, float nearPressure) {
	//	float weight = 1 - (dist / radius);
	//	return pressure * weight + nearPressure * weight * weight * 0.5f;
	//}


	//// Muller paper on enforcing incompressibility as a position constraint
	//void calculateLambda(unsigned int particleIndex);
	//void calculateDisplacement(unsigned int particleIndex);

	//// Clavet paper with double density kernel
	//void calculateDensity(unsigned int particleIndex);
	//void applyPressure(unsigned int particleIndex);

	//void applyBoundaryConstraints(unsigned int particleIndex);
	//void applyBoundaryPressure(unsigned int particleIndex);
};