#include "FluidSim.h"


void FluidSimSPH::addParticle(vec3 localPosition) {
	assert(particleCount < MAX_PARTICLES);

	previousPositions[particleCount] = localPosition + position;
	positions[particleCount] = localPosition + position;
	particleCount++;
}

// Spawns random particle within bounding box
void FluidSimSPH::spawnRandomParticles(unsigned int spawnCount) {
	assert(particleCount + spawnCount <= MAX_PARTICLES);

	for (int i = 0; i < spawnCount; i++) {
		vec3 randomPosition = glm::linearRand(position, position + bounds);

		previousPositions[particleCount] = randomPosition;
		positions[particleCount] = randomPosition;
		particleCount++;
	}
}


void FluidSimSPH::update(float deltaTime) {
	accumulatedTime += deltaTime;

	for (int step = 0; step < maxTicksPerUpdate && accumulatedTime > fixedTimeStep; step++) {
		tick();
		accumulatedTime -= fixedTimeStep;
	}
}

void FluidSimSPH::tick() {
	// Apply gravity
	for (unsigned int i = 0; i < particleCount; i++) {
		velocities[i] += gravity * fixedTimeStep;
	}

	// Project current and update previous particle positions
	for (unsigned int i = 0; i < particleCount; i++) {
		previousPositions[i] = positions[i];
		positions[i] += velocities[i] * fixedTimeStep;
	}

	// Build spatial hash grid with projected positions
	spatialHashGrid.generateHashTable(particleCount, positions);

	// Calculate and cache densities
	for (unsigned int i = 0; i < particleCount; i++) {
		calculateDensity(i);
	}

	// Apply pressure as displacement
	for (unsigned int i = 0; i < particleCount; i++) {
		// Issue: displacements do not occur in parallel and each iteration affects subsequent iterations
		// since there's a final radial distance check between particles
		applyPressure(i);
	}

	// Boundaries
	for (unsigned int i = 0; i < particleCount; i++) {
		applyBoundaryConstraints(i);
		applyBoundaryPressure(i);
	}


	// Compute implicit velocity
	for (unsigned int i = 0; i < particleCount; i++) {
		velocities[i] = (positions[i] - previousPositions[i]) / fixedTimeStep;
	}
}

void FluidSimSPH::calculateDensity(unsigned int particleIndex) {
	const unsigned int* hashTable = spatialHashGrid.getHashTable();
	const unsigned int* cellEntries = spatialHashGrid.getCellEntries();
	const unsigned int* cells = spatialHashGrid.getCells();

	const vec3& particlePos = positions[particleIndex];
	ivec3 cellCoords = spatialHashGrid.getCellCoords(particlePos);
	
	float density = 0.f;
	float nearDensity = 0.f;
	for (unsigned int i = 0; i < 27; i++) {
		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
		ivec3 offsetCellCoords = cellCoords + offset;
		
		unsigned int cellHash = spatialHashGrid.getCellHash(offsetCellCoords);
		unsigned int entries = cellEntries[cellHash];
		unsigned int cellIndex = hashTable[cellHash];

		for (unsigned int n = 0; n < entries; n++) {
			// The particle's local density should include its own influence so it doesn't matter if 'otherParticle' is itself.
			unsigned int otherParticle = cells[cellIndex * MAX_PARTICLES_PER_CELL + n];
			vec3 toParticle = positions[otherParticle] - particlePos;
			float sqrDist = dot(toParticle, toParticle);

			if (sqrDist > smoothingRadius * smoothingRadius) continue;

			float dist = sqrt(sqrDist);
			density += densityKernel(smoothingRadius, dist);
			nearDensity += nearDensityKernel(smoothingRadius, dist);
		}
	}

	densities[particleIndex] = density;
	nearDensities[particleIndex] = nearDensity;
}

void FluidSimSPH::applyPressure(unsigned int particleIndex) {
	const unsigned int* hashTable = spatialHashGrid.getHashTable();
	const unsigned int* cellEntries = spatialHashGrid.getCellEntries();
	const unsigned int* cells = spatialHashGrid.getCells();

	vec3& particlePos = positions[particleIndex];
	ivec3 cellCoords = spatialHashGrid.getCellCoords(particlePos);

	float pressure = calculatePressure(densities[particleIndex], restDensity, stiffness);
	float nearPressure = calculatePressure(nearDensities[particleIndex], 0, nearStiffness);

	vec3 pressureDisplacementSum = vec3(0);
	for (unsigned int i = 0; i < 27; i++) {
		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
		ivec3 offsetCellCoords = cellCoords + offset;

		unsigned int cellHash = spatialHashGrid.getCellHash(offsetCellCoords);
		unsigned int entries = cellEntries[cellHash];
		unsigned int cellIndex = hashTable[cellHash];

		for (unsigned int n = 0; n < entries; n++) {
			// The particle should not apply pressure to itself
			unsigned int otherParticle = cells[cellIndex * MAX_PARTICLES_PER_CELL + n];
			if (otherParticle == particleIndex) continue;

			vec3 toParticle = positions[otherParticle] - particlePos;
			float sqrDist = dot(toParticle, toParticle);

			if (sqrDist > smoothingRadius * smoothingRadius) continue;

			float dist = sqrt(sqrDist);
			vec3 unitDirection = (dist > 0) ? toParticle / dist : glm::sphericalRand(1.f);

			float pressureForce = calculatePressureForce(pressure, nearPressure, smoothingRadius, dist);
			vec3 pressureDisplacement = unitDirection * pressureForce * fixedTimeStep * fixedTimeStep;

			// assume mass = 1
			positions[otherParticle] += pressureDisplacement;
			pressureDisplacementSum -= pressureDisplacement;
		}
	}

	particlePos += pressureDisplacementSum;
}

void FluidSimSPH::applyBoundaryConstraints(unsigned int particleIndex) {
	vec3& particlePos = positions[particleIndex];

	vec3 boundsMin = position;
	vec3 boundsMax = position + bounds;

	particlePos = glm::clamp(particlePos, boundsMin, boundsMax);
}

void FluidSimSPH::applyBoundaryPressure(unsigned int particleIndex) {
	float artificialDensity = restDensity * 1.f;
	float pressure = artificialDensity * nearStiffness;

	vec3& particlePos = positions[particleIndex];

	vec3 boundsMin = position;
	vec3 boundsMax = position + bounds;
	// X-Axis
	if (particlePos.x + smoothingRadius > boundsMax.x) {
		float dist = boundsMax.x - particlePos.x;
		float value = 1 - (dist / smoothingRadius);

		particlePos.x -= pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
	else if (particlePos.x - smoothingRadius < boundsMin.x) {
		float dist = particlePos.x - boundsMin.x;
		float value = 1 - (dist / smoothingRadius);

		particlePos.x += pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
	// Y-Axis
	if (particlePos.y + smoothingRadius > boundsMax.y) {
		float dist = boundsMax.y - particlePos.y;
		float value = 1 - (dist / smoothingRadius);

		particlePos.y -= pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
	else if (particlePos.y - smoothingRadius < boundsMin.y) {
		float dist = particlePos.y - boundsMin.y;
		float value = 1 - (dist / smoothingRadius);

		particlePos.y += pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
	// Z-Axis
	if (particlePos.z + smoothingRadius > boundsMax.z) {
		float dist = boundsMax.z - particlePos.z;
		float value = 1 - (dist / smoothingRadius);

		particlePos.z -= pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
	else if (particlePos.z - smoothingRadius < boundsMin.z) {
		float dist = particlePos.z - boundsMin.z;
		float value = 1 - (dist / smoothingRadius);

		particlePos.z += pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
}



// For use with bounded spatial grid

//void FluidSimSPH::calculateDensity(int particleIndex) {
//	const ivec2* hashList = spatialGrid.getHashList();
//	const ivec2* lookupTable = spatialGrid.getLookupTable();
//
//	const vec3& projectedPos = projectedPositions[particleIndex];
//	ivec3 cellCoords = spatialGrid.getCellCoords(projectedPos);
//	
//	float density = 0.f;
//	float nearDensity = 0.f;
//	for (unsigned int i = 0; i < 27; i++) {
//		ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9);
//		ivec3 offsetCellCoords = cellCoords + offset - ivec3(1);
//		
//		if (!spatialGrid.isValidCoords(offsetCellCoords)) continue;
//
//		int cellHash = spatialGrid.getCellHash(offsetCellCoords);
//
//		int startIndex = lookupTable[cellHash].x;
//		int endIndex = lookupTable[cellHash].y;
//		if (startIndex == -1) continue; // Cell is empty
//
//		for (int n = startIndex; n < endIndex + 1; n++) {
//			// The particle's local density should include its own influence so it doesn't matter if 'otherParticle' is itself.
//			int otherParticle = hashList[n].x;
//			vec3 toParticle = projectedPositions[otherParticle] - projectedPos;
//			float sqrDist = dot(toParticle, toParticle);
//
//			if (sqrDist > smoothingRadius * smoothingRadius) continue;
//
//			float dist = sqrt(sqrDist);
//			density += densityKernel(smoothingRadius, dist);
//			nearDensity += nearDensityKernel(smoothingRadius, dist);
//		}
//	}
//
//	densities[particleIndex] = density;
//	nearDensities[particleIndex] = nearDensity;
//}
//
//void FluidSimSPH::applyPressure(int particleIndex) {
//	const ivec2* hashList = spatialGrid.getHashList();
//	const ivec2* lookupTable = spatialGrid.getLookupTable();
//
//	vec3& projectedPos = projectedPositions[particleIndex];
//	uvec3 cellCoords = spatialGrid.getCellCoords(projectedPos);
//
//	float pressure = calculatePressure(densities[particleIndex], restDensity, stiffness);
//	float nearPressure = calculatePressure(nearDensities[particleIndex], 0, nearStiffness);
//
//	vec3 pressureDisplacementSum = vec3(0);
//	for (unsigned int i = 0; i < 27; i++) {
//		uvec3 offset = uvec3(i % 3, (i / 3) % 3, i / 9);
//		uvec3 offsetCellCoords = cellCoords + offset - uvec3(1);
//
//		if (!spatialGrid.isValidCoords(offsetCellCoords)) continue;
//
//		unsigned int cellHash = spatialGrid.getCellHash(offsetCellCoords);
//
//		unsigned int startIndex = lookupTable[cellHash].x;
//		if (startIndex >= particleCount) continue; // Cell is empty
//
//		unsigned int endIndex = lookupTable[cellHash].y;
//
//		for (unsigned int n = startIndex; n < endIndex + 1; n++) {
//			// The particle should not apply pressure to itself
//			unsigned int otherParticle = hashList[n].x;
//			if (otherParticle == particleIndex) continue;
//
//			vec3 toParticle = projectedPositions[otherParticle] - projectedPos;
//			float sqrDist = dot(toParticle, toParticle);
//
//			if (sqrDist > smoothingRadius * smoothingRadius) continue;
//
//			float dist = sqrt(sqrDist);
//			vec3 unitDirection = (dist > 0) ? toParticle / dist : glm::sphericalRand(1.f);
//
//			float pressureForce = calculatePressureForce(pressure, nearPressure, smoothingRadius, dist);
//			vec3 pressureDisplacement = unitDirection * pressureForce * fixedTimeStep * fixedTimeStep;
//
//			// assume mass = 1
//			projectedPositions[otherParticle] += pressureDisplacement;
//			pressureDisplacementSum -= pressureDisplacement;
//		}
//	}
//
//	projectedPos += pressureDisplacementSum;
//}