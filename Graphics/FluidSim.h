#pragma once

#include <algorithm>

#include "ParticleManager.h"


using glm::uvec2;
using glm::uvec3;


class SpatialGrid {
private:
	vec3 bounds;
	float cellSize;

	uvec3 gridBounds;
	unsigned int cellCount;

	// (position index, cell hash)
	unsigned int hashListSize = 0;
	uvec2 hashList[MAX_PARTICLES];

	// (start index, end index)
	uvec2* lookupTable = nullptr;
	//unsigned int lookupTableSize = 0;

public:
	SpatialGrid() {}
	~SpatialGrid() {
		delete[] lookupTable;
	}

	void init(vec3 _bounds, float _cellSize) {
		bounds = _bounds;
		cellSize = _cellSize;

		gridBounds = uvec3(ceil(_bounds / _cellSize));
		cellCount = gridBounds.x * gridBounds.y * gridBounds.z;

		lookupTable = new uvec2[cellCount];
	}

	uvec2* getHashList() {
		return hashList;
	}

	uvec2* getLookupTable() {
		return lookupTable;
	}

	bool isValidCoords(uvec3 cellCoords) {
		return cellCoords.x < gridBounds.x && cellCoords.y < gridBounds.y && cellCoords.z < gridBounds.z;
	}

	// returns grid coordinates of cell that 'pos' falls into
	uvec3 getCellCoords(vec3 pos) {
		uvec3 coords = uvec3(floor(pos / cellSize));
		coords = glm::min(coords, gridBounds - uvec3(1));

		return coords;
	}

	unsigned int getCellHash(uvec3 cellCoords) {
		unsigned int hash = cellCoords.x + cellCoords.y * gridBounds.x + cellCoords.z * gridBounds.x * gridBounds.y;
		//hash = (hash >= cellCount) ? cellCount - 1 : hash; // clamps (unsigned can't be less than 0)

		return hash;
	}

	void generateHashList(unsigned int count, const vec3* positions) {
		assert(count <= MAX_PARTICLES);
		
		hashListSize = count;

		for (int i = 0; i < count; i++) {
			hashList[i] = uvec2(i, getCellHash(getCellCoords(positions[i])));
		}
	}


	void sortHashList() {
		std::sort(hashList, &hashList[hashListSize - 1] + 1, [](const uvec2& a, const uvec2& b) {return a.y < b.y;});
	}

	void generateLookupTable() {
		assert(hashListSize > 0);

		for (int i = 0; i < cellCount; i++) {
			lookupTable[i] = uvec2(-1, -1);
		}

		int currentStart = 0;
		int previousCellHash = -1;
		for (int i = 0; i < hashListSize - 1; i++) {
			if (hashList[i].y == hashList[i + 1].y) {
				continue;
			}

			lookupTable[hashList[i].y].x = currentStart;

			if (previousCellHash != -1) {
				lookupTable[previousCellHash].y = currentStart - 1;
			}
			previousCellHash = hashList[i].y;

			currentStart = i + 1;
		}

		if (previousCellHash != -1) {
			lookupTable[previousCellHash].y = currentStart - 1;
		}

		lookupTable[hashList[hashListSize - 1].y].x = currentStart;
		lookupTable[hashList[hashListSize - 1].y].y = hashListSize - 1;
	}
};



// SPH Fluid simulation within a bounding box
class FluidSimSPH {
private:
	SpatialGrid spatialGrid;

	vec3 position;
	vec3 bounds;

	const int maxTicksPerUpdate = 5;
	const float fixedTimeStep = 0.01f;
	float accumulatedTime = 0.f;

	vec3 gravity;
	float particleRadius; // displayed radius (doesn't really get used in simulation)
	float smoothingRadius; // radius of influence
	float restDensity;
	float pressureMultiplier;
	float nearPressureMultiplier;

	unsigned int particleCount = 0;
	vec3 positions[MAX_PARTICLES];
	vec3 projectedPositions[MAX_PARTICLES];
	vec3 velocities[MAX_PARTICLES];
	float densities[MAX_PARTICLES];
	float nearDensities[MAX_PARTICLES];


public:
	FluidSimSPH() {}
	~FluidSimSPH() {}

	void init(vec3 _position, vec3 _bounds, vec3 _gravity, float _particleRadius = 0.25f,
		float _restDensity = 2.5f, float _pressureMultiplier = 100, float _nearPressureMultiplier = 200) {
		
		float _smoothingRadius = _particleRadius * 2;

		spatialGrid.init(_bounds, _smoothingRadius);

		position = _position;
		bounds = _bounds;

		gravity = _gravity;
		particleRadius = _particleRadius;
		smoothingRadius = _smoothingRadius;
		restDensity = _restDensity;
		pressureMultiplier = _pressureMultiplier;
		nearPressureMultiplier = _nearPressureMultiplier;
	}

	void clearParticles() { particleCount = 0; }
	void addParticle(vec3 localPosition);
	void spawnRandomParticle();

	void update(float deltaTime);
	void tick();

	void transferData(ParticleManager& particleManager) {
		particleManager.setParticleRadius(particleRadius);

		particleManager.clearParticles();
		for (int i = 0; i < particleCount; i++) {
			particleManager.addParticle(positions[i] + position);
		}
	}

private:
	static float densityKernel(float radius, float dist) {
		float value = 1 - (dist / radius);
		return value * value;
	}

	static float nearDensityKernel(float radius, float dist) {
		float value = 1 - (dist / radius);
		return value * value * value;
	}

	void calculateDensity(int particleIndex);

	void applyPressure(int particleIndex);

	void applyBoundaryPressure(int particleIndex);
};