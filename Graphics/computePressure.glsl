#version 430 core

#include "common.h"


layout(local_size_x = COMPUTE_CELLS_PER_WORKGROUP, local_size_y = MAX_PARTICLES_PER_CELL, local_size_z = 1) in;


layout(binding = FLUID_CONFIG_UBO, std140) uniform FluidConfig {
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

layout(binding = FLUID_DATA_SSBO, std430) restrict buffer FluidData {
	vec4 positions[MAX_PARTICLES];
	readonly vec4 previousPositions[MAX_PARTICLES];
	readonly vec4 velocities[MAX_PARTICLES];
	//writeonly vec4 pressureDisplacements[MAX_PARTICLES]; // This one
	readonly float densities[MAX_PARTICLES];
	readonly float nearDensities[MAX_PARTICLES];

	readonly uint usedCells;
	uint hashes[MAX_PARTICLES];
	readonly uint hashTable[MAX_PARTICLES];
	readonly uint cellEntries[MAX_PARTICLES];
	readonly uint cells[];
} data;


uniform uint time;

// Random function
vec3 randVec(uint index) {
	const uint p1 = 585533779;
	const uint p2 = 1926125923;

	uint x = (time * p1) ^ (time ^ (p2 * index));
	uint y = time ^ ((x >> 10) * p1);
	uint z = (x * p1) ^ (y * p2) * time;

	return vec3(uintBitsToFloat(x), uintBitsToFloat(y), uintBitsToFloat(z));
}



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


// // Calculates pressure displacements caused by specified particle
// void calculatePressureDisplacements(uint particleIndex) {
// 	ivec3 cellCoords = getCellCoords(data.positions[particleIndex].xyz);

// 	float sqrSmoothingRadius = config.smoothingRadius * config.smoothingRadius;

// 	float pressure = calculatePressure(data.densities[particleIndex], config.restDensity, config.stiffness);
// 	float nearPressure = calculatePressure(data.nearDensities[particleIndex], 0, config.nearStiffness);

// 	vec3 pressureDisplacementSum = vec3(0);
// 	for (unsigned int i = 0; i < 27; i++) {
// 		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
// 		ivec3 offsetCellCoords = cellCoords + offset;

// 		unsigned int cellHash = getCellHash(offsetCellCoords);
// 		//uint entries = data.cellEntries[cellHash];
// 		uint cellIndex = data.hashTable[cellHash];
// 		if(cellIndex == 0xFFFFFFFF) continue;

// 		uint entries = data.cellEntries[cellIndex];

// 		for (unsigned int n = 0; n < entries; n++) {
// 			uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + n;
// 			unsigned int otherParticleIndex = data.cells[cellEntryIndex];
// 			if (otherParticleIndex == particleIndex) continue;

// 			vec3 toParticle = data.positions[otherParticleIndex].xyz - data.positions[particleIndex].xyz;
// 			float sqrDist = dot(toParticle, toParticle);

// 			if (sqrDist > sqrSmoothingRadius) continue;

// 			float dist = sqrt(sqrDist);
// 			vec3 unitDirection = (dist > 0) ? toParticle / dist : normalize(randVec(particleIndex % gl_GlobalInvocationID.x));

// 			// assume mass = 1
// 			float pressureForce = calculatePressureForce(pressure, nearPressure, config.smoothingRadius, dist);
// 			vec3 pressureDisplacement = unitDirection * pressureForce * config.timeStep * config.timeStep;

// 			data.pressureDisplacements[otherParticleIndex].xyz += pressureDisplacement;
// 			pressureDisplacementSum -= pressureDisplacement;
// 		}
// 	}

// 	data.pressureDisplacements[particleIndex].xyz += pressureDisplacementSum;
// }

void applyBoundaryConstraints(uint particleIndex) {
	vec3 particlePos = data.positions[particleIndex].xyz;
	data.positions[particleIndex].xyz = clamp(particlePos, config.boundsMin.xyz, config.boundsMax.xyz);
}

void applyBoundaryPressure(uint particleIndex) {
	float artificialDensity = config.restDensity * 1.f;
	float pressure = artificialDensity * config.nearStiffness;

	vec3 particlePos = data.positions[particleIndex].xyz;

	vec3 boundsMin = config.boundsMin.xyz;
	vec3 boundsMax = config.boundsMax.xyz;
	// X-Axis
	if (particlePos.x + config.smoothingRadius > boundsMax.x) {
		float dist = boundsMax.x - particlePos.x;
		float value = 1 - (dist / config.smoothingRadius);

		particlePos.x -= pressure * value * value * config.timeStep * config.timeStep;
	}
	else if (particlePos.x - config.smoothingRadius < boundsMin.x) {
		float dist = particlePos.x - boundsMin.x;
		float value = 1 - (dist / config.smoothingRadius);

		particlePos.x += pressure * value * value * config.timeStep * config.timeStep;
	}
	// Y-Axis
	if (particlePos.y + config.smoothingRadius > boundsMax.y) {
		float dist = boundsMax.y - particlePos.y;
		float value = 1 - (dist / config.smoothingRadius);

		particlePos.y -= pressure * value * value * config.timeStep * config.timeStep;
	}
	else if (particlePos.y - config.smoothingRadius < boundsMin.y) {
		float dist = particlePos.y - boundsMin.y;
		float value = 1 - (dist / config.smoothingRadius);

		particlePos.y += pressure * value * value * config.timeStep * config.timeStep;
	}
	// Z-Axis
	if (particlePos.z + config.smoothingRadius > boundsMax.z) {
		float dist = boundsMax.z - particlePos.z;
		float value = 1 - (dist / config.smoothingRadius);

		particlePos.z -= pressure * value * value * config.timeStep * config.timeStep;
	}
	else if (particlePos.z - config.smoothingRadius < boundsMin.z) {
		float dist = particlePos.z - boundsMin.z;
		float value = 1 - (dist / config.smoothingRadius);

		particlePos.z += pressure * value * value * config.timeStep * config.timeStep;
	}

	data.positions[particleIndex].xyz = particlePos;
}


void main() {
	uint cellIndex = gl_GlobalInvocationID.x;
	uint entryIndex = gl_LocalInvocationID.y;
	if(cellIndex >= data.usedCells || entryIndex >= data.cellEntries[cellIndex]) {
		return;
	}

	ivec3 cellCoords = getCellCoords(data.positions[data.cells[cellIndex * MAX_PARTICLES_PER_CELL]].xyz);

	uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + entryIndex;
	uint particleIndex = data.cells[cellEntryIndex];

	float pressure = calculatePressure(data.densities[particleIndex], config.restDensity, config.stiffness);
	float nearPressure = calculatePressure(data.nearDensities[particleIndex], 0, config.nearStiffness);

	vec3 pressureDisplacementSum = vec3(0);
	for (uint i = 0; i < 27; i++) {
		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
		ivec3 offsetCellCoords = cellCoords + offset;

		unsigned int cellHash = getCellHash(offsetCellCoords);

		uint offsetCellIndex = data.hashTable[cellHash];
		if(offsetCellIndex == 0xFFFFFFFF) continue;

		uint entries = data.cellEntries[offsetCellIndex];

		for (uint n = 0; n < entries; n++) {
			uint offsetCellEntryIndex = offsetCellIndex * MAX_PARTICLES_PER_CELL + n;
			unsigned int otherParticleIndex = data.cells[offsetCellEntryIndex];
			if (otherParticleIndex == particleIndex) continue;

			vec3 toParticle = data.positions[otherParticleIndex].xyz - data.positions[particleIndex].xyz;
			float sqrDist = dot(toParticle, toParticle);

			if (sqrDist > config.smoothingRadius * config.smoothingRadius) continue;

			float dist = sqrt(sqrDist);
			vec3 unitDirection = (dist > 0) ? toParticle / dist : normalize(randVec(particleIndex * gl_GlobalInvocationID.x));

			float otherPressure = calculatePressure(data.densities[otherParticleIndex], config.restDensity, config.stiffness);
			float otherNearPressure = calculatePressure(data.nearDensities[otherParticleIndex], 0, config.nearStiffness);
			
			// assume mass = 1
			float pressureForce = calculatePressureForce(pressure, nearPressure, config.smoothingRadius, dist);
			float otherPressureForce = calculatePressureForce(otherPressure, otherNearPressure, config.smoothingRadius, dist);

			vec3 pressureDisplacement = unitDirection * (pressureForce + otherPressureForce) * config.timeStep * config.timeStep;

			//data.pressureDisplacements[otherParticleIndex].xyz += pressureDisplacement;
			pressureDisplacementSum -= pressureDisplacement;
		}
	}

	//data.pressureDisplacements[particleIndex].xyz = pressureDisplacementSum;

	// Apply pressure displacements
	data.positions[particleIndex].xyz += pressureDisplacementSum;
	//data.positions[particleIndex].xyz += data.pressureDisplacements[particleIndex].xyz;
	//memoryBarrierBuffer();

	// Boundaries
	applyBoundaryConstraints(particleIndex);
	applyBoundaryPressure(particleIndex);

	// Compute implicit velocity
	data.velocities[particleIndex].xyz = (data.positions[particleIndex].xyz - data.previousPositions[particleIndex].xyz) / config.timeStep;
}