// MathLibrary.h - Contains declarations of math functions
#pragma once

#ifdef MODULARFLUIDSLIBRARY_EXPORTS
#define MODULARFLUIDSLIBRARY_API __declspec(dllexport)
#else
#define MODULARFLUIDSLIBRARY_API __declspec(dllimport)
#endif

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>

#define MAX_PARTICLES 2048
//#define MAX_PARTICLES 262144

//#define MAX_PARTICLES_PER_CELL 16 // Only viable when using Mullet.M position based fluid technique
#define MAX_PARTICLES_PER_CELL 32

#define WORKGROUP_SIZE_X 1024

#define COMPUTE_CELLS_PER_WORKGROUP 16

#define PROJECTIONVIEW_UBO 0

#define FLUID_CONFIG_UBO 1
#define FLUID_DATA_SSBO 2


// The Fibonacci recurrence relation describes a sequence F
// where F(n) is { n = 0, a
//               { n = 1, b
//               { n > 1, F(n-2) + F(n-1)
// for some initial integral values a and b.
// If the sequence is initialized F(0) = 1, F(1) = 1,
// then this relation produces the well-known Fibonacci
// sequence: 1, 1, 2, 3, 5, 8, 13, 21, 34, ...

//// Initialize a Fibonacci relation sequence
//// such that F(0) = a, F(1) = b.
//// This function must be called before any other function.
//extern "C" MODULARFLUIDSLIBRARY_API void fibonacci_init(
//    const unsigned long long a, const unsigned long long b);
//
//// Produce the next value in the sequence.
//// Returns true on success and updates current value and index;
//// false on overflow, leaves current value and index unchanged.
//extern "C" MODULARFLUIDSLIBRARY_API bool fibonacci_next();
//
//// Get the current value in the sequence.
//extern "C" MODULARFLUIDSLIBRARY_API unsigned long long fibonacci_current();
//
//// Get the position of the current value in the sequence.
//extern "C" MODULARFLUIDSLIBRARY_API unsigned fibonacci_index();


using glm::uvec2;
using glm::uvec3;
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::uvec4;


class UBO {
private:
	unsigned int ubo_id = 0;

public:
	UBO() {}
	~UBO() { glDeleteBuffers(1, &ubo_id); }

	void init(GLsizeiptr size) {
		assert(ubo_id == 0 && "Shader storage buffer already initialized");

		glGenBuffers(1, &ubo_id);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
		glBufferData(GL_UNIFORM_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void subData(GLintptr offset, GLsizeiptr size, const void* data) {
		glBindBuffer(GL_UNIFORM_BUFFER, ubo_id);
		glBufferSubData(GL_UNIFORM_BUFFER, offset, size, data);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void bindBufferBase(GLuint bindingIndex) {
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, ubo_id);
	}
};


class SSBO {
private:
	unsigned int ssbo_id = 0;

public:
	SSBO() {}
	~SSBO() { glDeleteBuffers(1, &ssbo_id); }

	void init(GLsizeiptr size) {
		assert(ssbo_id == 0 && "Shader storage buffer already initialized");

		glGenBuffers(1, &ssbo_id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, size, NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void subData(GLintptr offset, GLsizeiptr size, const void* data) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void clearNamedSubData(GLenum internalFormat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data) {
		glClearNamedBufferSubData(ssbo_id, internalFormat, offset, size, format, type, data);
	}

	void getSubData(GLintptr offset, GLsizeiptr size, void* data) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo_id);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void bindBufferBase(GLuint bindingIndex) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, ssbo_id);
	}
};

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
	unsigned int hashTable[MAX_PARTICLES];// clear this one
	unsigned int cellEntries[MAX_PARTICLES];// clear this one
	unsigned int cells[MAX_PARTICLES * MAX_PARTICLES_PER_CELL];// clear this one
};

class SPHCompute {
private:
	vec3 position = vec3(0);
    vec3 bounds = vec3(1);

	const float fixedTimeStep = 0.01f;

	vec3 gravity = vec3(0, 0, -1);
	float smoothingRadius;
	float restDensity;
	float particleMass;

	float stiffness;
	float nearStiffness;

	unsigned int particleCount = 0;
	float particleRadius;

	UBO configUBO;
	SSBO particleSSBO;

	// Buffer for particle position data.
	vec4 positionBuffer[1024];

public:
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

		// Clavet.S parameters
		stiffness = _stiffness;
		nearStiffness = _nearStiffness;

		// Buffers
		configUBO.init(sizeof(configUBO));
		syncUBO();

		GLsizeiptr sizePerParticle = sizeof(vec4) * 3 + sizeof(float) * 6 + sizeof(float) * MAX_PARTICLES_PER_CELL;
		particleSSBO.init(MAX_PARTICLES * sizePerParticle + sizeof(unsigned int));
		resetHashDataSSBO();
	}

	// Stores simulation parameters in a buffer and then sends buffer data to GPU.
	void syncUBO() {
		uboData tempBuffer = {
			vec4(position, 0),
			vec4(position + bounds, 0),

			vec4(gravity, 0),
			smoothingRadius,
			restDensity,
			particleMass,

			stiffness,
			nearStiffness,

			fixedTimeStep,
			particleCount
		};

		configUBO.subData(0, sizeof(uboData), &tempBuffer);
	}

	void resetHashDataSSBO() {
		unsigned int usedCells = 0;
		unsigned int defaultHash = 0xFFFFFFFF;
		particleSSBO.clearNamedSubData(GL_R32UI, 15 * MAX_PARTICLES * sizeof(float), sizeof(float), GL_RED_INTEGER, GL_UNSIGNED_INT, &usedCells);
		particleSSBO.clearNamedSubData(GL_R32UI, ((16 * MAX_PARTICLES) + 1) * sizeof(float), MAX_PARTICLES * sizeof(float), GL_RED_INTEGER, GL_UNSIGNED_INT, &defaultHash);
		particleSSBO.clearNamedSubData(GL_R32UI, ((17 * MAX_PARTICLES) + 1) * sizeof(float), MAX_PARTICLES * sizeof(float), GL_RED_INTEGER, GL_UNSIGNED_INT, &usedCells);
		//glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);
	}

	unsigned int getParticleCount() { return particleCount; }

	void clearParticles() { particleCount = 0; }

	// Spawns particles randomly within simulation bounds in batches of 1024.
	void spawnRandomParticles(unsigned int spawnCount) {
		assert(particleCount + spawnCount <= MAX_PARTICLES);

		unsigned int i = 0;
		while (i < spawnCount) {
			unsigned int batchCount = 0;
			while (batchCount < 1024 && i < spawnCount) {
				vec3 randomPosition = glm::linearRand(position, position + bounds);
				positionBuffer[batchCount] = vec4(randomPosition, 0);

				batchCount++;
				i++;
			}

			// Fill position and previous position memory chunk.
			particleSSBO.subData((particleCount) * sizeof(vec4), batchCount * sizeof(vec4), positionBuffer);
			particleSSBO.subData((MAX_PARTICLES + particleCount) * sizeof(vec4), batchCount * sizeof(vec4), positionBuffer);

			particleCount += batchCount;
		}

		syncUBO();
	}


	void bindConfigUBO(GLuint bindingIndex) {
		configUBO.bindBufferBase(bindingIndex);
	}

	void bindParticleSSBO(GLuint bindingIndex) {
		particleSSBO.bindBufferBase(bindingIndex);
	}
};