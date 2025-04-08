#include "FluidSim.h"


void FluidSimSPH::addParticle(vec3 localPosition) {
	assert(particleCount < MAX_PARTICLES);

	positions[particleCount] = localPosition;
	projectedPositions[particleCount] = localPosition;
	particleCount++;
}

// Spawns random particle within bounding box
void FluidSimSPH::spawnRandomParticle() {
	assert(particleCount < MAX_PARTICLES);

	vec3 randomPosition = glm::linearRand(vec3(0), bounds);

	positions[particleCount] = randomPosition;
	projectedPositions[particleCount] = randomPosition;
	particleCount++;
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
	for (int i = 0; i < particleCount; i++) {
		velocities[i] += gravity * fixedTimeStep;
	}

	// Advance and project particle positions
	for (int i = 0; i < particleCount; i++) {
		positions[i] = projectedPositions[i];
		projectedPositions[i] += velocities[i] * fixedTimeStep;
	}

	// Build spatial hash grid with projected positions
	spatialGrid.buildSpatialGrid(particleCount, projectedPositions);
	//spatialGrid.generateHashList(particleCount, projectedPositions);
	//spatialGrid.sortHashList();
	//spatialGrid.generateLookupTable();

	// Calculate and cache densities
	for (int i = 0; i < particleCount; i++) {
		calculateDensity(i);
	}

	// Apply pressure as displacement
	for (int i = 0; i < particleCount; i++) {
		applyPressure(i);
	}

	// Boundaries
	for (int i = 0; i < particleCount; i++) {
		applyBoundaryPressure(i);
	}

	// Compute implicit velocity
	for (int i = 0; i < particleCount; i++) {
		velocities[i] = (projectedPositions[i] - positions[i]) / fixedTimeStep;
	}
}


void FluidSimSPH::calculateDensity(int particleIndex) {
	const vec3& projectedPos = projectedPositions[particleIndex];

	const uvec2* hashList = spatialGrid.getHashList();
	const uvec2* lookupTable = spatialGrid.getLookupTable();

	uvec3 cellCoords = spatialGrid.getCellCoords(projectedPos);

	// particle's density should include itself
	float density = 0.f;
	float nearDensity = 0.f;
	for (int i = 0; i < 27; i++) {
		unsigned int x = i % 3;
		unsigned int y = (i / 3) % 3;
		unsigned int z = i / 9;

		uvec3 offset = uvec3(x, y, z);
		uvec3 offsetCellCoords = cellCoords + offset - uvec3(1);
		
		if (!spatialGrid.isValidCoords(offsetCellCoords)) continue;

		unsigned int cellHash = spatialGrid.getCellHash(offsetCellCoords);

		unsigned int startIndex = lookupTable[cellHash].x;
		if (startIndex >= particleCount) {
			continue;
		}

		unsigned int endIndex = lookupTable[cellHash].y;

		for (unsigned int n = startIndex; n < endIndex + 1; n++) {
			unsigned int otherParticle = hashList[n].x;
			vec3 toParticle = projectedPositions[otherParticle] - projectedPos;
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

void FluidSimSPH::applyPressure(int particleIndex) {
	vec3& projectedPos = projectedPositions[particleIndex];

	const uvec2* hashList = spatialGrid.getHashList();
	const uvec2* lookupTable = spatialGrid.getLookupTable();

	uvec3 cellCoords = spatialGrid.getCellCoords(projectedPos);

	vec3 pressureDisplacementSum = vec3(0);
	for (int i = 0; i < 27; i++) {
		unsigned int x = i % 3;
		unsigned int y = (i / 3) % 3;
		unsigned int z = i / 9;

		uvec3 offset = uvec3(x, y, z);
		uvec3 offsetCellCoords = cellCoords + offset - uvec3(1);

		if (!spatialGrid.isValidCoords(offsetCellCoords)) continue;

		unsigned int cellHash = spatialGrid.getCellHash(offsetCellCoords);

		unsigned int startIndex = lookupTable[cellHash].x;
		if (startIndex >= particleCount) {
			continue;
		}

		unsigned int endIndex = lookupTable[cellHash].y;

		for (unsigned int n = startIndex; n < endIndex + 1; n++) {
			unsigned int otherParticle = hashList[n].x;
			if (otherParticle == particleIndex) continue;

			vec3 toParticle = projectedPositions[otherParticle] - projectedPos;
			float sqrDist = dot(toParticle, toParticle);

			if (sqrDist > smoothingRadius * smoothingRadius) continue;

			float dist = sqrt(sqrDist);
			vec3 unitDirection = (dist > 0) ? toParticle / dist : glm::sphericalRand(1.f);

			float pressure = (densities[otherParticle] - restDensity) * pressureMultiplier;
			float nearPressure = nearDensities[otherParticle] * nearPressureMultiplier;

			float weight = 1 - (dist / smoothingRadius);

			float sharedPressure = (pressure * weight + nearPressure * weight * weight);
			vec3 pressureDisplacement = unitDirection * (sharedPressure * 0.5f) * fixedTimeStep * fixedTimeStep;

			// assume mass = 1
			projectedPositions[otherParticle] += pressureDisplacement;
			pressureDisplacementSum -= pressureDisplacement;
		}
	}

	projectedPos += pressureDisplacementSum;
}

void FluidSimSPH::applyBoundaryPressure(int particleIndex) {
	float artificialDensity = restDensity;
	float pressure = artificialDensity * nearPressureMultiplier;

	vec3& particlePos = projectedPositions[particleIndex];
	// X-Axis
	if (particlePos.x + smoothingRadius > bounds.x) {
		float dist = bounds.x - particlePos.x;
		float value = 1 - (dist / smoothingRadius);

		particlePos.x -= pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
	else if (particlePos.x - smoothingRadius < 0) {
		float dist = particlePos.x;
		float value = 1 - (dist / smoothingRadius);

		particlePos.x += pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
	// Y-Axis
	if (particlePos.y + smoothingRadius > bounds.y) {
		float dist = bounds.y - particlePos.y;
		float value = 1 - (dist / smoothingRadius);

		particlePos.y -= pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
	else if (particlePos.y - smoothingRadius < 0) {
		float dist = particlePos.y;
		float value = 1 - (dist / smoothingRadius);

		particlePos.y += pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
	// Z-Axis
	if (particlePos.z + smoothingRadius > bounds.z) {
		float dist = bounds.z - particlePos.z;
		float value = 1 - (dist / smoothingRadius);

		particlePos.z -= pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
	else if (particlePos.z - smoothingRadius < 0) {
		float dist = particlePos.z;
		float value = 1 - (dist / smoothingRadius);

		particlePos.z += pressure * value * value * fixedTimeStep * fixedTimeStep;
	}
}