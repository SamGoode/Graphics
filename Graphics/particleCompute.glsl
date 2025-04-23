#version 430 core

#include "common.h"


layout(local_size_x = WORKGROUP_SIZE_X, local_size_y = 1, local_size_z = 1) in;


layout(binding = 1, std140) uniform FluidConfig {
	vec4 boundsMin;
	vec4 boundsMax;
	vec4 gravity;
	float smoothingRadius;
	float restDensity;
	float stiffness;
	float nearStiffness;
	
	float timeStep;
	uint particleCount;
} config;

layout(binding = 2, std430) coherent restrict buffer FluidData {
	vec4 positions[MAX_PARTICLES];
	vec4 previousPositions[MAX_PARTICLES];
	vec4 velocities[MAX_PARTICLES];
	vec4 pressureDisplacements[MAX_PARTICLES];
	float densities[MAX_PARTICLES];
	float nearDensities[MAX_PARTICLES];

	uint usedCells;
	uint hashTable[MAX_PARTICLES];
	uint cellEntries[MAX_PARTICLES];
	uint cells[MAX_PARTICLES * MAX_PARTICLES_PER_CELL];
} data;


// Spatial hashing
ivec3 getCellCoords(vec3 point) {
	return ivec3(floor(point / config.smoothingRadius));
}

uint getCellHash(ivec3 cellCoords) {
	// Three large prime numbers (from the brain of Matthias Teschner)
	const uint p1 = 73856093;
	const uint p2 = 19349663; // Apparently this one isn't prime
	const uint p3 = 83492791;

	return ((p1 * uint(cellCoords.x)) ^ (p2 * uint(cellCoords.y)) ^ (p3 * uint(cellCoords.z))) % MAX_PARTICLES;
}


// Density kernels
float densityKernel(float radius, float dist) {
	float value = 1 - (dist / radius);
	return value * value;
}

float nearDensityKernel(float radius, float dist) {
	float value = 1 - (dist / radius);
	return value * value * value;
}


// Pressure
float calculatePressure(float density, float restDensity, float stiffness) {
	return (density - restDensity) * stiffness;
}

float calculatePressureForce(float pressure, float nearPressure, float radius, float dist) {
	float weight = 1 - (dist / radius);
	return pressure * weight + nearPressure * weight * weight * 0.5f;
}


// Calculates density at specified particle position
void calculateDensity(uint particleIndex) {
	ivec3 cellCoords = getCellCoords(data.positions[particleIndex].xyz);
	
	float sqrSmoothingRadius = config.smoothingRadius * config.smoothingRadius;

	float density = 0.f;
	float nearDensity = 0.f;
	for (uint i = 0; i < 27; i++) {
		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
		ivec3 offsetCellCoords = cellCoords + offset;
		
		uint cellHash = getCellHash(offsetCellCoords);
		uint entries = data.cellEntries[cellHash];
		uint cellIndex = data.hashTable[cellHash];

		for (uint n = 0; n < entries; n++) {
			uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + n;
			uint otherParticleIndex = data.cells[cellEntryIndex];

			vec3 toParticle = data.positions[otherParticleIndex].xyz - data.positions[particleIndex].xyz;
			float sqrDist = dot(toParticle, toParticle);

			if (sqrDist > sqrSmoothingRadius) continue;

			float dist = sqrt(sqrDist);
			density += densityKernel(config.smoothingRadius, dist);
			nearDensity += nearDensityKernel(config.smoothingRadius, dist);
		}
	}

	data.densities[particleIndex] = density;
	data.nearDensities[particleIndex] = nearDensity;
}


// Calculates pressure displacements caused by specified particle
void calculatePressureDisplacements(uint particleIndex) {
	ivec3 cellCoords = getCellCoords(data.positions[particleIndex].xyz);

	float sqrSmoothingRadius = config.smoothingRadius * config.smoothingRadius;

	float pressure = calculatePressure(data.densities[particleIndex], config.restDensity, config.stiffness);
	float nearPressure = calculatePressure(data.nearDensities[particleIndex], 0, config.nearStiffness);

	vec3 pressureDisplacementSum = vec3(0);
	for (unsigned int i = 0; i < 27; i++) {
		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
		ivec3 offsetCellCoords = cellCoords + offset;

		unsigned int cellHash = getCellHash(offsetCellCoords);
		unsigned int entries = data.cellEntries[cellHash];
		unsigned int cellIndex = data.hashTable[cellHash];

		for (unsigned int n = 0; n < entries; n++) {
			uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + n;
			unsigned int otherParticleIndex = data.cells[cellEntryIndex];
			if (otherParticleIndex == particleIndex) continue;

			vec3 toParticle = data.positions[otherParticleIndex].xyz - data.positions[particleIndex].xyz;
			float sqrDist = dot(toParticle, toParticle);

			if (sqrDist > sqrSmoothingRadius) continue;

			float dist = sqrt(sqrDist);
			vec3 unitDirection = (dist > 0) ? toParticle / dist : vec3(0, 0, 1);//glm::sphericalRand(1.f);

			// assume mass = 1
			float pressureForce = calculatePressureForce(pressure, nearPressure, config.smoothingRadius, dist);
			vec3 pressureDisplacement = unitDirection * pressureForce * config.timeStep * config.timeStep;

			data.pressureDisplacements[otherParticleIndex].xyz += pressureDisplacement;
			pressureDisplacementSum -= pressureDisplacement;
		}
	}

	data.pressureDisplacements[particleIndex].xyz += pressureDisplacementSum;
}

void applyBoundaryConstraints(uint particleIndex) {
	vec3 particlePos = data.positions[particleIndex].xyz;
	data.positions[particleIndex].xyz = clamp(particlePos, config.boundsMin.xyz, config.boundsMax.xyz);
}


//shared uint usedCells;

void main() {
	uint particleIndex = gl_GlobalInvocationID.x;
	if(particleIndex >= config.particleCount) {
		return;
	}

	// Apply gravity
	data.velocities[particleIndex].xyz += config.gravity.xyz * config.timeStep;

	// Project current and update previous particle positions
	data.previousPositions[particleIndex].xyz = data.positions[particleIndex].xyz;
	//data.positions[particleIndex].xyz += data.velocities[particleIndex].xyz * config.timeStep;


	uint cellHash = getCellHash(getCellCoords(data.positions[particleIndex].xyz));

	uint assignedCellIndex = atomicAdd(data.usedCells, uint(data.hashTable[cellHash] == 0xFFFFFFFF));
    atomicCompSwap(data.hashTable[cellHash], 0xFFFFFFFF, assignedCellIndex);

	//uint cellEntryCount = atomicAdd(data.cellEntries[cellHash], 1);
	//uint assignedCellIndex = atomicAdd(data.usedCells, uint(cellEntryCount == 0));
	//atomicCompSwap(data.hashTable[cellHash], data.hashTable[cellHash] + cellEntryCount, assignedCellIndex);
	//uint value = atomicExchange(data.hashTable[cellHash], assignedCellIndex);
	//uint value = data.hashTable[cellHash];
	//uint oldValue = atomicCompSwap(data.hashTable[cellHash], data.hashTable[cellHash] + cellEntryCount, assignedCellIndex);


//	// Memory barrier needed here
//
//	calculateDensity(particleIndex);
//
//	// Memory barrier needed here
//	// pressureDisplacements cache is shared
//
//	calculatePressureDisplacements(particleIndex);
//
//	// Memory barrier
//
//	// Apply pressure displacements
//	positions[particleIndex].xyz += pressureDisplacements[particleIndex].xyz;

	// Boundaries
	//applyBoundaryConstraints(particleIndex);
	//applyBoundaryPressure(particleIndex);

	// Compute implicit velocity
	//data.velocities[particleIndex] = (data.positions[particleIndex] - data.previousPositions[particleIndex]) / config.timeStep;
}