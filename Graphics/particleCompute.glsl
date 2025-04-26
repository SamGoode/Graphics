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
	vec4 pressureDisplacements[MAX_PARTICLES];
	float densities[MAX_PARTICLES];
	float nearDensities[MAX_PARTICLES];

	uint usedCells;
	coherent uint hashTable[MAX_PARTICLES];
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



void main() {
	uint particleIndex = gl_GlobalInvocationID.x;
	if(particleIndex >= config.particleCount) {
		return;
	}

	// Apply gravity
	data.velocities[particleIndex].xyz += config.gravity.xyz * config.timeStep;

	// Project current and update previous particle positions
	data.previousPositions[particleIndex].xyz = data.positions[particleIndex].xyz;
	data.positions[particleIndex].xyz += data.velocities[particleIndex].xyz * config.timeStep;


	uint cellHash = getCellHash(getCellCoords(data.positions[particleIndex].xyz));

	uint hashStatus = atomicCompSwap(data.hashTable[cellHash], 0xFFFFFFFF, 0x8FFFFFFF);

	bool shouldAssignNewCell = (hashStatus == 0xFFFFFFFF);

	uint assignedCellIndex = atomicAdd(data.usedCells, uint(shouldAssignNewCell));
	if(!shouldAssignNewCell) {
		assignedCellIndex = 0x8FFFFFFF;
	}

	uint cellIndex = atomicCompSwap(data.hashTable[cellHash], 0x8FFFFFFF, assignedCellIndex);
	memoryBarrierBuffer();
	while(cellIndex == 0x8FFFFFFF) {
		cellIndex = data.hashTable[cellHash];
	}
    
	uint cellEntryCount = atomicAdd(data.cellEntries[cellIndex], 1);

	uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + cellEntryCount;
	
	data.cells[cellEntryIndex] = particleIndex;

	atomicMax(indirectCmd.num_groups_x, (data.usedCells / COMPUTE_CELLS_PER_WORKGROUP) + uint((data.usedCells % COMPUTE_CELLS_PER_WORKGROUP) != 0));
	//atomicMax(indirectCmd.num_groups_x, data.usedCells);
	indirectCmd.num_groups_y = 1;
	indirectCmd.num_groups_z = 1;

	// Boundaries
	//applyBoundaryConstraints(particleIndex);
	//applyBoundaryPressure(particleIndex);

	// Compute implicit velocity
	//data.velocities[particleIndex] = (data.positions[particleIndex] - data.previousPositions[particleIndex]) / config.timeStep;
}