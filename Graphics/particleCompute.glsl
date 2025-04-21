#version 430 core

#define MAX_PARTICLES 1024
#define MAX_PARTICLES_PER_CELL 16


layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;

//layout(std140) uniform PVMatrices {
//	mat4 View;
//	mat4 Projection;
//	mat4 ViewInverse;
//	mat4 ProjectionInverse;
//};

//layout(binding = 1, std430) readonly restrict buffer FluidSimSSBO {
//	uint particleCount;
//	float smoothingRadius;
//	vec2 padding;
//	vec4 positions[MAX_PARTICLES];
//	uint hashTable[MAX_PARTICLES];
//	uint cellEntries[MAX_PARTICLES];
//	uint cells[MAX_PARTICLES * MAX_PARTICLES_PER_CELL];
//};

layout(std140) uniform FluidConfig {
	vec4 gravity;
	float smoothingRadius;
	float restDensity;
	float stiffness;
	float nearStiffness;
	
	float timeStep;
	uint particleCount;
} config;

layout(binding = 2, std430) buffer FluidSim {
	vec4 previousPositions[MAX_PARTICLES];
	vec4 positions[MAX_PARTICLES];
	vec4 velocities[MAX_PARTICLES];
	vec4 pressureDisplacements[MAX_PARTICLES];
	float densities[MAX_PARTICLES];
	float nearDensities[MAX_PARTICLES];

	uint usedCells;
	uint hashTable[MAX_PARTICLES];
	uint cellEntries[MAX_PARTICLES];
	uint cells[MAX_PARTICLES * MAX_PARTICLES_PER_CELL];
};


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
	ivec3 cellCoords = getCellCoords(positions[particleIndex].xyz);
	
	float sqrSmoothingRadius = config.smoothingRadius * config.smoothingRadius;

	float density = 0.f;
	float nearDensity = 0.f;
	for (uint i = 0; i < 27; i++) {
		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
		ivec3 offsetCellCoords = cellCoords + offset;
		
		uint cellHash = getCellHash(offsetCellCoords);
		uint entries = cellEntries[cellHash];
		uint cellIndex = hashTable[cellHash];

		for (uint n = 0; n < entries; n++) {
			uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + n;
			uint otherParticleIndex = cells[cellEntryIndex];

			vec3 toParticle = positions[otherParticleIndex].xyz - positions[particleIndex].xyz;
			float sqrDist = dot(toParticle, toParticle);

			if (sqrDist > sqrSmoothingRadius) continue;

			float dist = sqrt(sqrDist);
			density += densityKernel(config.smoothingRadius, dist);
			nearDensity += nearDensityKernel(config.smoothingRadius, dist);
		}
	}

	densities[particleIndex] = density;
	nearDensities[particleIndex] = nearDensity;
}


// Calculates pressure displacements caused by specified particle
void calculatePressureDisplacements(uint particleIndex) {
	ivec3 cellCoords = getCellCoords(positions[particleIndex].xyz);

	float sqrSmoothingRadius = config.smoothingRadius * config.smoothingRadius;

	float pressure = calculatePressure(densities[particleIndex], config.restDensity, config.stiffness);
	float nearPressure = calculatePressure(nearDensities[particleIndex], 0, config.nearStiffness);

	vec3 pressureDisplacementSum = vec3(0);
	for (unsigned int i = 0; i < 27; i++) {
		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
		ivec3 offsetCellCoords = cellCoords + offset;

		unsigned int cellHash = getCellHash(offsetCellCoords);
		unsigned int entries = cellEntries[cellHash];
		unsigned int cellIndex = hashTable[cellHash];

		for (unsigned int n = 0; n < entries; n++) {
			uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + n;
			unsigned int otherParticleIndex = cells[cellEntryIndex];
			if (otherParticleIndex == particleIndex) continue;

			vec3 toParticle = positions[otherParticleIndex].xyz - positions[particleIndex].xyz;
			float sqrDist = dot(toParticle, toParticle);

			if (sqrDist > sqrSmoothingRadius) continue;

			float dist = sqrt(sqrDist);
			vec3 unitDirection = (dist > 0) ? toParticle / dist : vec3(0, 0, 1);//glm::sphericalRand(1.f);

			// assume mass = 1
			float pressureForce = calculatePressureForce(pressure, nearPressure, config.smoothingRadius, dist);
			vec3 pressureDisplacement = unitDirection * pressureForce * config.timeStep * config.timeStep;

			pressureDisplacements[otherParticleIndex].xyz += pressureDisplacement;
			pressureDisplacementSum -= pressureDisplacement;
		}
	}

	pressureDisplacements[particleIndex].xyz += pressureDisplacementSum;
}

void applyBoundaryConstraints(uint particleIndex, vec3 boundsMin, vec3 boundsMax) {
	vec3 particlePos = positions[particleIndex].xyz;
	positions[particleIndex].xyz = clamp(particlePos, boundsMin, boundsMax);
}


void main() {
	uint particleIndex = gl_GlobalInvocationID.x;
	if(particleIndex >= config.particleCount) {
		return;
	}

	// Apply gravity
	velocities[particleIndex].xyz += config.gravity.xyz * config.timeStep;

	// Project current and update previous particle positions
	previousPositions[particleIndex].xyz = positions[particleIndex].xyz;
	positions[particleIndex].xyz += velocities[particleIndex].xyz * config.timeStep;


	// Clear cellEntries
	usedCells = 0;
	cellEntries[gl_GlobalInvocationID.x] = 0;

	// Memory barrier needed here
	// usedCells and cellEntries are shared
	// hashTable and cells are also shared

	uint cellHash = getCellHash(getCellCoords(positions[particleIndex].xyz));

	if(cellEntries[cellHash] == 0) {
		hashTable[cellHash] = usedCells; // Allocates new 'memory'
		usedCells++;
	}

	uint cellIndex = hashTable[cellHash];
	uint cellEntryIndex = cellIndex * MAX_PARTICLES_PER_CELL + cellEntries[cellHash];
	
	cells[cellEntryIndex] = particleIndex;
	cellEntries[cellHash]++;

	// Memory barrier needed here

	calculateDensity(particleIndex);

	// Memory barrier needed here
	// pressureDisplacements cache is shared

	calculatePressureDisplacements(particleIndex);

	// Memory barrier

	// Apply pressure displacements
	positions[particleIndex].xyz += pressureDisplacements[particleIndex].xyz;

	// Boundaries
	//applyBoundaryConstraints(particleIndex, simPosition.xyz, simPosition.xyz + simBounds.xyz);
	//applyBoundaryPressure(particleIndex);

	// Compute implicit velocity
	velocities[particleIndex] = (positions[particleIndex] - previousPositions[particleIndex]) / config.timeStep;
}