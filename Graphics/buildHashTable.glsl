#version 430 core

#include "common.h"


layout(local_size_x = WORKGROUP_SIZE_X, local_size_y = 1, local_size_z = 1) in;


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
	vec4 previousPositions[MAX_PARTICLES];
	vec4 velocities[MAX_PARTICLES];
	//vec4 pressureDisplacements[MAX_PARTICLES];
	float densities[MAX_PARTICLES];
	float nearDensities[MAX_PARTICLES];

	uint usedCells;
	uint hashes[MAX_PARTICLES];
	uint hashTable[MAX_PARTICLES];
	uint cellEntries[MAX_PARTICLES];
	uint cells[];
} data;

layout(binding = 3, std430) restrict buffer DispatchIndirectCommand {
	uint num_groups_x;
	uint num_groups_y;
	uint num_groups_z;
} indirectCmd;


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
    uint particleIndex = gl_GlobalInvocationID.x;
	if(particleIndex >= config.particleCount) {
		return;
	}

    uint cellHash = data.hashes[particleIndex];
    uint cellIndex = data.hashTable[cellHash];

	uint cellEntryCount = atomicAdd(data.cellEntries[cellIndex], 1);
	uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + cellEntryCount;
	
	data.cells[cellEntryIndex] = particleIndex;

	// if(cellHash == data.hashes[particleIndex]) {
	// 	// early out
	// 	return;
	// }

	// uint previousCellHash = data.hashes[particleIndex];
	// data.hashes[particleIndex] = cellHash;

	// if(previousCellHash != 0xFFFFFFFF) {
	// 	uint previousCellIndex = data.hashTable[previousCellHash];
	// 	uint entryCount = atomicAdd(data.cellEntries[previousCellIndex], -1);

	// 	uint previousCellEntryIndex = 0xFFFFFFFF;
	// 	for(uint i = 0; i < entryCount; i++) {
	// 		previousCellEntryIndex = previousCellIndex * MAX_PARTICLES_PER_CELL + i;
	// 		if(particleIndex == data.cells[previousCellEntryIndex]) {
	// 			break;
	// 		}
	// 	}

	// 	uint lastEntryIndex = entryCount - 1;
	// 	uint lastEntryValue = data.cells[previousCellIndex * MAX_PARTICLES_PER_CELL + lastEntryIndex];

	// 	data.cells[previousCellIndex * MAX_PARTICLES_PER_CELL + previousCellEntryIndex] = lastEntryValue;
	// 	// 1 2 3 4
	// 	// 1 2 4 4
	// 	// 1 3 
	// 	//atomic
	// }

	//atomicMax(indirectCmd.num_groups_x, data.usedCells);
	//atomicMax(indirectCmd.num_groups_x, (data.usedCells / COMPUTE_CELLS_PER_WORKGROUP) + uint((data.usedCells % COMPUTE_CELLS_PER_WORKGROUP) != 0));
	indirectCmd.num_groups_x = (data.usedCells / COMPUTE_CELLS_PER_WORKGROUP) + uint((data.usedCells % COMPUTE_CELLS_PER_WORKGROUP) != 0);
    indirectCmd.num_groups_y = 1;
	indirectCmd.num_groups_z = 1;
}