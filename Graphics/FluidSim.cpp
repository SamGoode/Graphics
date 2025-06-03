#include "FluidSim.h"


void FluidSimSPH::addParticle(vec3 localPosition) {
	assert(particleCount < MAX_PARTICLES);

	vec4 pos = vec4(localPosition + position, 0);
	previousPositions[particleCount] = pos;
	positions[particleCount] = pos;
	particleCount++;
}

// Spawns random particle within bounding box
void FluidSimSPH::spawnRandomParticles(unsigned int spawnCount) {
	assert(particleCount + spawnCount <= MAX_PARTICLES);

	for (unsigned int i = 0; i < spawnCount; i++) {
		vec3 randomPosition = glm::linearRand(position, position + bounds);

		positions[particleCount + i] = vec4(randomPosition, 0);
		previousPositions[particleCount + i] = vec4(randomPosition, 0);
	}

	if (isSimGPU) {
		particleSSBO.subData(particleCount * sizeof(vec4), spawnCount * sizeof(vec4), &particleSSBO.buffer.positions[particleCount]);
		particleSSBO.subData((MAX_PARTICLES + particleCount) * sizeof(vec4), spawnCount * sizeof(vec4), &particleSSBO.buffer.previousPositions[particleCount]);
	}

	particleCount += spawnCount;


	configUBO.buffer.particleCount = particleCount;
	configUBO.subData();
}


void FluidSimSPH::update(float deltaTime) {
	accumulatedTime += deltaTime;

	for (unsigned int step = 0; step < maxTicksPerUpdate && accumulatedTime > fixedTimeStep; step++) {
		accumulatedTime -= fixedTimeStep;

		if (isSimGPU) {
			tickSimGPU();
		}
		else {
			tickSimCPU();
		}
	}
}

void FluidSimSPH::tickSimGPU() {
	unsigned int usedCells = 0;
	particleSSBO.subData(15 * MAX_PARTICLES * sizeof(float), sizeof(float), &usedCells);
	particleSSBO.subData(((16 * MAX_PARTICLES) + 1) * sizeof(float), MAX_PARTICLES * sizeof(float), particleSSBO.buffer.hashTable);
	particleSSBO.subData(((17 * MAX_PARTICLES) + 1) * sizeof(float), MAX_PARTICLES * sizeof(float), particleSSBO.buffer.cellEntries);
	glMemoryBarrier(GL_BUFFER_UPDATE_BARRIER_BIT);

	dispatchIndirect.bindToIndex(3);
	dispatchIndirect.clear();


	
	particleComputeShader.use();
	glDispatchCompute((particleCount / WORKGROUP_SIZE_X) + ((particleCount % WORKGROUP_SIZE_X) != 0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	computeHashTableShader.use();
	glDispatchCompute((particleCount / WORKGROUP_SIZE_X) + ((particleCount % WORKGROUP_SIZE_X) != 0), 1, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	dispatchIndirect.bind();
	glMemoryBarrier(GL_COMMAND_BARRIER_BIT);

	for (unsigned int iteration = 0; iteration < solverIterations; iteration++) {
		computeDensityShader.use();
		glDispatchComputeIndirect(0);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		computePressureShader.use();
		int time = (int)std::time(0);
		computePressureShader.bindUniform(time, "time");
		glDispatchComputeIndirect(0);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}
}

void FluidSimSPH::tickSimCPU() {
	// Compute implicit velocity
	for (unsigned int i = 0; i < particleCount; i++) {
		velocities[i] = (positions[i] - previousPositions[i]) / fixedTimeStep;
	}

	// Update previous particle positions
	for (unsigned int i = 0; i < particleCount; i++) {
		previousPositions[i] = positions[i];
	}

	// Apply gravity
	for (unsigned int i = 0; i < particleCount; i++) {
		velocities[i] += vec4(gravity * fixedTimeStep, 0);
	}

	// Project current particle positions
	for (unsigned int i = 0; i < particleCount; i++) {
		positions[i] += velocities[i] * fixedTimeStep;
	}


	//// Boundaries
	//for (unsigned int i = 0; i < particleCount; i++) {
	//	applyBoundaryConstraints(i);
	//	//applyBoundaryPressure(i);
	//}


	// Build spatial hash grid with projected positions
	spatialHashGrid.generateHashTable(particleCount, positions);


	for (unsigned int iteration = 0; iteration < solverIterations; iteration++) {
		for (unsigned int particleIndex = 0; particleIndex < particleCount; particleIndex++) {
			calculateLambda(particleIndex);
		}

		for (unsigned int particleIndex = 0; particleIndex < particleCount; particleIndex++) {
			calculateDisplacement(particleIndex);
		}

		for (unsigned int particleIndex = 0; particleIndex < particleCount; particleIndex++) {
			positions[particleIndex] += vec4(displacements[particleIndex], 0);
		}

		// Boundaries
		for (unsigned int particleIndex = 0; particleIndex < particleCount; particleIndex++) {
			applyBoundaryConstraints(particleIndex);
		}
	}
}


void FluidSimSPH::calculateLambda(unsigned int particleIndex) {
	vec3 particlePos = vec3(positions[particleIndex]);
	ivec3 cellCoords = spatialHashGrid.getCellCoords(particlePos);

	float localDensity = 0.f;
	float localDensityGradient = 0.f;

	float constraintGradient = 0.f;
	auto func = [&](unsigned int otherParticleIndex) {
		vec3 toParticle = vec3(positions[otherParticleIndex]) - particlePos;
		float sqrDist = dot(toParticle, toParticle);

		if (sqrDist > sqrSmoothingRadius) return;

		localDensity += particleMass * polySixKernelSqr(sqrDist, sqrSmoothingRadius, normFactor_P6);

		float dist = sqrt(sqrDist);

		float densityDerivative = particleMass * spikyKernelGradient(dist, smoothingRadius, normFactor_S);
		localDensityGradient += densityDerivative;

		if (particleIndex == otherParticleIndex) return;

		constraintGradient -= densityDerivative * densityDerivative;
	};

	spatialHashGrid.iterate3x3x3(cellCoords, func);

	constraintGradient += (localDensityGradient * localDensityGradient);
	constraintGradient /= (restDensity * restDensity);

	float densityConstraint = (localDensity / restDensity) - 1.f;

	float lambda = -densityConstraint / (constraintGradient + epsilon);
	lambdas[particleIndex] = lambda;
}

void FluidSimSPH::calculateDisplacement(unsigned int particleIndex) {
	vec3 particlePos = vec3(positions[particleIndex]);
	ivec3 cellCoords = spatialHashGrid.getCellCoords(particlePos);

	float lambda = lambdas[particleIndex];

	vec3 displacement = vec3(0);
	auto func = [&](unsigned int otherParticleIndex) {
		if (particleIndex == otherParticleIndex) return;

		vec3 toParticle = vec3(positions[otherParticleIndex]) - particlePos;
		float sqrDist = dot(toParticle, toParticle);

		if (sqrDist > sqrSmoothingRadius) return;

		float otherLambda = lambdas[otherParticleIndex];
		
		float density = particleMass * polySixKernelSqr(sqrDist, sqrSmoothingRadius, normFactor_P6);
		float correction = 0.f;//-k * (float)glm::pow((density / densityDeltaQ), N);

		float dist = sqrt(sqrDist);
		vec3 unitDir = dist > 0 ? toParticle / dist : glm::sphericalRand(1.f);
		
		displacement += unitDir * (lambda + otherLambda + correction) * particleMass * spikyKernelGradient(dist, smoothingRadius, normFactor_S);
	};

	spatialHashGrid.iterate3x3x3(cellCoords, func);

	displacements[particleIndex] = displacement / restDensity;
}


void FluidSimSPH::calculateDensity(unsigned int particleIndex) {
	const vec3 particlePos = positions[particleIndex];
	ivec3 cellCoords = spatialHashGrid.getCellCoords(particlePos);
	
	float density = 0.f;
	float nearDensity = 0.f;

	auto func = [&](unsigned int otherParticleIndex) {
		vec3 toParticle = vec3(positions[otherParticleIndex]) - particlePos;
		float sqrDist = dot(toParticle, toParticle);

		if (sqrDist > sqrSmoothingRadius) return;

		float dist = sqrt(sqrDist);
		density += densityKernel(dist, smoothingRadius);
		nearDensity += nearDensityKernel(dist, smoothingRadius);
	};

	spatialHashGrid.iterate3x3x3(cellCoords, func);

	densities[particleIndex] = density;
	nearDensities[particleIndex] = nearDensity;
}

void FluidSimSPH::applyPressure(unsigned int particleIndex) {
	vec3 particlePos = positions[particleIndex];
	ivec3 cellCoords = spatialHashGrid.getCellCoords(particlePos);

	float pressure = calculatePressure(densities[particleIndex], restDensity, stiffness);
	float nearPressure = calculatePressure(nearDensities[particleIndex], 0, nearStiffness);

	vec3 pressureDisplacementSum = vec3(0);

	auto func = [&](unsigned int otherParticleIndex) {
		// The particle should not apply pressure to itself
		if (otherParticleIndex == particleIndex) return;

		vec3 toParticle = vec3(positions[otherParticleIndex]) - particlePos;
		float sqrDist = dot(toParticle, toParticle);

		if (sqrDist > sqrSmoothingRadius) return;

		float dist = sqrt(sqrDist);
		vec3 unitDirection = (dist > 0) ? toParticle / dist : glm::sphericalRand(1.f);

		float pressureForce = calculatePressureForce(dist, smoothingRadius, pressure, nearPressure);
		vec3 pressureDisplacement = unitDirection * pressureForce * fixedTimeStep * fixedTimeStep;

		// assume mass = 1
		positions[otherParticleIndex] += vec4(pressureDisplacement, 0);
		pressureDisplacementSum -= pressureDisplacement;
	};

	spatialHashGrid.iterate3x3x3(cellCoords, func);

	//particlePos += pressureDisplacementSum;
	positions[particleIndex] += vec4(pressureDisplacementSum, 0);
}



void FluidSimSPH::applyBoundaryConstraints(unsigned int particleIndex) {
	vec3 particlePos = positions[particleIndex];

	vec3 boundsMin = position;
	vec3 boundsMax = position + bounds;

	positions[particleIndex] = vec4(glm::clamp(particlePos, boundsMin + vec3(smoothingRadius), boundsMax - vec3(smoothingRadius)), 0);
}

void FluidSimSPH::applyBoundaryPressure(unsigned int particleIndex) {
	float artificialDensity = restDensity * 1.f;
	float pressure = artificialDensity * nearStiffness;

	vec3 particlePos = positions[particleIndex];

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

	positions[particleIndex] = vec4(particlePos, 1);
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