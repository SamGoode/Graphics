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
	readonly vec4 positions[MAX_PARTICLES];
	readonly vec4 previousPositions[MAX_PARTICLES];
	readonly vec4 velocities[MAX_PARTICLES];
	readonly vec4 pressureDisplacements[MAX_PARTICLES];
	writeonly float densities[MAX_PARTICLES]; // These
	writeonly float nearDensities[MAX_PARTICLES]; // Ones

	readonly uint usedCells;
	readonly uint hashTable[MAX_PARTICLES];
	readonly uint cellEntries[MAX_PARTICLES];
	readonly uint cells[];
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
		uint cellIndex = data.hashTable[cellHash];
		if(cellIndex == 0xFFFFFFFF) continue;

		uint entries = data.cellEntries[cellIndex];


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

void applyBoundaryConstraints(uint particleIndex) {
	vec3 particlePos = data.positions[particleIndex].xyz;
	data.positions[particleIndex].xyz = clamp(particlePos, config.boundsMin.xyz, config.boundsMax.xyz);
}


//shared ivec3 cellCoords;

//shared float densityCache[MAX_PARTICLES_PER_CELL];
//shared float nearDensityCache[MAX_PARTICLES_PER_CELL];


void main() {
	//uint cellIndex = gl_WorkGroupID.x;
	uint cellIndex = gl_GlobalInvocationID.x;

	ivec3 cellCoords = getCellCoords(data.positions[data.cells[cellIndex * MAX_PARTICLES_PER_CELL]].xyz);

//	densityCache[gl_LocalInvocationID.x] = 0;
//	nearDensityCache[gl_LocalInvocationID.x] = 0;

//	if(gl_LocalInvocationID.y == 0) {
//		uint particleIndex = data.cells[cellIndex * MAX_PARTICLES_PER_CELL];
//		cellCoords = getCellCoords(data.positions[particleIndex].xyz);
//	}
//	memoryBarrierShared();
//	barrier();

	uint entryIndex = gl_LocalInvocationID.y;

	bool isValidThread = (cellIndex < data.usedCells && entryIndex < data.cellEntries[cellIndex]);
	if (!isValidThread) return;


	uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + entryIndex;
	uint particleIndex = data.cells[cellEntryIndex];

	float density = 0.f;
	float nearDensity = 0.f;
	for (uint i = 0; i < 27 && isValidThread; i++) {
		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
		ivec3 offsetCellCoords = cellCoords + offset;
		
		uint cellHash = getCellHash(offsetCellCoords);

		uint offsetCellIndex = data.hashTable[cellHash];
		if(offsetCellIndex == 0xFFFFFFFF) continue;

		uint entries = data.cellEntries[offsetCellIndex];

//		uint n = gl_LocalInvocationID.y;
//		if(n >= entries) continue;

		for (uint n = 0; n < entries; n++) {
			uint offsetCellEntryIndex = offsetCellIndex * MAX_PARTICLES_PER_CELL + n;
			uint otherParticleIndex = data.cells[offsetCellEntryIndex];

			vec3 toParticle = data.positions[otherParticleIndex].xyz - data.positions[particleIndex].xyz;
			float sqrDist = dot(toParticle, toParticle);

			if (sqrDist > config.smoothingRadius * config.smoothingRadius) continue;

			float dist = sqrt(sqrDist);
			density += densityKernel(config.smoothingRadius, dist);
			nearDensity += nearDensityKernel(config.smoothingRadius, dist);
//			densityCache[n] += densityKernel(config.smoothingRadius, dist);
//			nearDensityCache[n] += nearDensityKernel(config.smoothingRadius, dist);
		}
	}

//	memoryBarrierShared();
//	barrier();


	//if(!isValidThread || gl_LocalInvocationID.y != 0) return;

	data.densities[particleIndex] = density;
	data.nearDensities[particleIndex] = nearDensity;

//	float total = 0.0;
//	float nearTotal = 0.0;
//	for(int i = 0; i < MAX_PARTICLES_PER_CELL; i++) {
//		total += densityCache[i];
//		nearTotal += nearDensityCache[i];
//	}
//
//	data.densities[particleIndex] = total;
//	data.nearDensities[particleIndex] = nearTotal;
}