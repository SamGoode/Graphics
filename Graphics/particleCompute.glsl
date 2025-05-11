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

	readonly float lambdas[MAX_PARTICLES];
	readonly float densities[MAX_PARTICLES];
	readonly float nearDensities[MAX_PARTICLES];

	uint usedCells;
	writeonly uint hashes[MAX_PARTICLES];
	uint hashTable[MAX_PARTICLES];
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


// Boundary
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
	if(particleIndex >= config.particleCount) return;


	//---------(From previous timestep)--------------
	// // Apply pressure displacements
	// data.positions[particleIndex].xyz += data.pressureDisplacements[particleIndex].xyz;
	// data.pressureDisplacements[particleIndex].xyz = vec3(0); // reset
	//------------------------------------------------

	// Boundaries
	applyBoundaryConstraints(particleIndex);
	//applyBoundaryPressure(particleIndex);

	// Compute implicit velocity
	data.velocities[particleIndex].xyz = (data.positions[particleIndex].xyz - data.previousPositions[particleIndex].xyz) / config.timeStep;

	// Update previous particle position
	data.previousPositions[particleIndex].xyz = data.positions[particleIndex].xyz;

	// Apply gravity and other external forces
	data.velocities[particleIndex].xyz += config.gravity.xyz * config.timeStep;

	// Project current particle position
	data.positions[particleIndex].xyz += data.velocities[particleIndex].xyz * config.timeStep;

	

	// Contiguously assign indexes to populated cells
	uint cellHash = getCellHash(getCellCoords(data.positions[particleIndex].xyz));
	data.hashes[particleIndex] = cellHash;

	// this could probably be replaced with atomicExchange
	uint hashStatus = atomicCompSwap(data.hashTable[cellHash], 0xFFFFFFFF, 0x8FFFFFFF);

	// Cell Hash Status
	// 0xFFFFFFFF : unassigned
	// 0x8FFFFFFF : pending assignment

	// Assign index to cell hash if new
	bool shouldAssignNewCell = (hashStatus == 0xFFFFFFFF);
	uint assignedCellIndex = atomicAdd(data.usedCells, uint(shouldAssignNewCell));

	if(shouldAssignNewCell)
		data.hashTable[cellHash] = assignedCellIndex;
}