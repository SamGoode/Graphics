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

	readonly float lambdas[MAX_PARTICLES]; // This one
	readonly float densities[MAX_PARTICLES];
	readonly float nearDensities[MAX_PARTICLES];

	readonly uint usedCells;
	readonly uint hashes[MAX_PARTICLES];
	readonly uint hashTable[MAX_PARTICLES];
	readonly uint cellEntries[MAX_PARTICLES];
	readonly uint cells[];
} data;


uniform uint time;

const float sqrSmoothingRadius = config.smoothingRadius * config.smoothingRadius;


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


// Mullen.M
// Kernel normalization factors
const float normFactor_P6 = 315/(64 * acos(-1) * pow(config.smoothingRadius, 9));
const float normFactor_S = 45 / (acos(-1) * pow(config.smoothingRadius, 6));

// Density kernels
float polySixKernel(float sqrDist) {
	float value = sqrSmoothingRadius - sqrDist;
	return value * value * value * normFactor_P6;
}

float spikyKernelGradient(float dist) {
	float value = config.smoothingRadius - dist;
	return value * value * normFactor_S;
}

// Correction term parameters
const float k = 0.0000005f;
const int N = 4;
const float deltaQ = 0.3f * config.smoothingRadius;
const float densityAtDeltaQ = polySixKernel(deltaQ * deltaQ);

// Calculates displacement (∆p) to solve density constraint
void calculateDisplacement(uint particleIndex, out vec3 displacement) {
 	ivec3 cellCoords = getCellCoords(data.positions[particleIndex].xyz);

	float lambda = data.lambdas[particleIndex];

 	displacement = vec3(0);
 	for (uint i = 0; i < 27; i++) {
 		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
 		ivec3 offsetCellCoords = cellCoords + offset;

 		uint cellHash = getCellHash(offsetCellCoords);
 		uint cellIndex = data.hashTable[cellHash];
 		if(cellIndex == 0xFFFFFFFF) continue;

 		uint entries = data.cellEntries[cellIndex];

 		for (uint n = 0; n < entries; n++) {
 			uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + n;
 			uint otherParticleIndex = data.cells[cellEntryIndex];
 			if (particleIndex == otherParticleIndex) continue;

 			vec3 toParticle = data.positions[otherParticleIndex].xyz - data.positions[particleIndex].xyz;
 			float sqrDist = dot(toParticle, toParticle);

 			if (sqrDist > sqrSmoothingRadius) continue;

			float density = polySixKernel(sqrDist);
			float correctionTerm = -k * pow((density / densityAtDeltaQ), N);

			float otherLambda = data.lambdas[otherParticleIndex];

 			float dist = sqrt(sqrDist);
 			vec3 unitDir = (dist > 0) ? toParticle / dist : normalize(randVec(particleIndex * gl_GlobalInvocationID.x));

			displacement += unitDir * (lambda + otherLambda + correctionTerm) * spikyKernelGradient(dist);
 		}
	}

	displacement /= config.restDensity;
}


// Clavet.S
// Pressure conversion
float calculatePressure(float density, float restDensity, float stiffness) {
	return (density - restDensity) * stiffness;
}

float calculatePressureForce(float pressure, float nearPressure, float radius, float dist) {
	float weight = 1 - (dist / radius);
	return pressure * weight + nearPressure * weight * weight * 0.5f;
}

// Calculates pressure displacements caused by specified particle
void calculatePressureDisplacement(uint particleIndex, out vec3 pressureDisplacement) {
 	ivec3 cellCoords = getCellCoords(data.positions[particleIndex].xyz);

 	float pressure = calculatePressure(data.densities[particleIndex], config.restDensity, config.stiffness);
 	float nearPressure = calculatePressure(data.nearDensities[particleIndex], 0, config.nearStiffness);

 	pressureDisplacement = vec3(0);
 	for (uint i = 0; i < 27; i++) {
 		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
 		ivec3 offsetCellCoords = cellCoords + offset;

 		uint cellHash = getCellHash(offsetCellCoords);
 		uint cellIndex = data.hashTable[cellHash];
 		if(cellIndex == 0xFFFFFFFF) continue;

 		uint entries = data.cellEntries[cellIndex];

 		for (uint n = 0; n < entries; n++) {
 			uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + n;
 			uint otherParticleIndex = data.cells[cellEntryIndex];
 			if (particleIndex == otherParticleIndex) continue;

 			vec3 toParticle = data.positions[otherParticleIndex].xyz - data.positions[particleIndex].xyz;
 			float sqrDist = dot(toParticle, toParticle);

 			if (sqrDist > sqrSmoothingRadius) continue;

 			float dist = sqrt(sqrDist);
 			vec3 unitDirection = (dist > 0) ? toParticle / dist : normalize(randVec(particleIndex * gl_GlobalInvocationID.x));

			float otherPressure = calculatePressure(data.densities[otherParticleIndex], config.restDensity, config.stiffness);
			float otherNearPressure = calculatePressure(data.nearDensities[otherParticleIndex], 0, config.nearStiffness);
			
			// assume mass = 1
			float pressureForce = calculatePressureForce(pressure, nearPressure, config.smoothingRadius, dist);
			float otherPressureForce = calculatePressureForce(otherPressure, otherNearPressure, config.smoothingRadius, dist);

			vec3 displacement = unitDirection * (pressureForce + otherPressureForce) * config.timeStep * config.timeStep;

 			pressureDisplacement -= displacement;
 		}
	}
}


void main() {
	uint cellIndex = gl_GlobalInvocationID.x;
	uint entryIndex = gl_LocalInvocationID.y;

	bool isValidThread = (cellIndex < data.usedCells && entryIndex < data.cellEntries[cellIndex]);
	if (!isValidThread) return;

	uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + entryIndex;
	uint particleIndex = data.cells[cellEntryIndex];


	// Calculate and apply pressure displacement
	vec3 displacement;
	
	// Mullet.M
	calculateDisplacement(particleIndex, displacement);
	// Clavet.S
	//calculatePressureDisplacement(particleIndex, displacement);

	data.positions[particleIndex].xyz += displacement;
}