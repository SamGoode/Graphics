#pragma once

#include <algorithm>

#include <glm/ext.hpp>

using glm::ivec2;
using glm::ivec3;
using glm::uvec2;
using glm::uvec3;
using glm::vec3;


class SpatialGrid {
private:
	vec3 bounds;
	float cellSize;

	ivec3 gridBounds;
	unsigned int cellCount;

	unsigned int entries = 0;
	unsigned int capacity;

	// (position index, cell hash)
	ivec2* hashList = nullptr;
	// (start index, end index)
	ivec2* lookupTable = nullptr;


public:
	SpatialGrid() {}
	~SpatialGrid() {
		delete[] hashList;
		delete[] lookupTable;
	}

	void init(vec3 _bounds, float _cellSize, unsigned int _capacity) {
		bounds = _bounds;
		cellSize = _cellSize;

		gridBounds = ivec3(ceil(_bounds / _cellSize));
		cellCount = gridBounds.x * gridBounds.y * gridBounds.z;

		capacity = _capacity;
		hashList = new ivec2[capacity];
		lookupTable = new ivec2[cellCount]{ ivec2(-1) };

		for (int i = 0; i < cellCount; i++) {
			lookupTable[i] = ivec2(-1);
		}
	}

	glm::ivec3 getGridBounds() { return gridBounds; }
	unsigned int getCellCount() { return cellCount; }
	const ivec2* getHashList() { return hashList; }
	const ivec2* getLookupTable() { return lookupTable; }

	// returns grid coordinates of cell that 'position' falls into
	ivec3 getCellCoords(vec3 position) { 
		//return ivec3(floor(glm::clamp(position, vec3(0), bounds) / cellSize));
		return glm::clamp(ivec3(floor(position / cellSize)), ivec3(0), gridBounds - ivec3(1));
	}
	bool isValidCoords(ivec3 cellCoords) { 
		return cellCoords.x >= 0 && cellCoords.y >= 0 && cellCoords.z >= 0 && cellCoords.x < gridBounds.x && cellCoords.y < gridBounds.y && cellCoords.z < gridBounds.z;
	}

	unsigned int getCellHash(const uvec3& cellCoords) {
		assert(isValidCoords(cellCoords) && "Invalid cell coordinates");
		
		return cellCoords.x + cellCoords.y * gridBounds.x + cellCoords.z * gridBounds.x * gridBounds.y;
	}

	void buildSpatialGrid(unsigned int count, const vec3* positions) {
		generateHashList(count, positions);
		sortHashList();
		generateLookupTable();
	}

private:
	void generateHashList(unsigned int count, const vec3* positions) {
		assert(count <= capacity);

		entries = count;
		for (int i = 0; i < entries; i++) {
			hashList[i] = ivec2(i, getCellHash(getCellCoords(positions[i])));
		}
	}

	void sortHashList() { std::sort(&hashList[0], &hashList[entries - 1] + 1, [](const ivec2& a, const ivec2& b) {return a.y < b.y;}); }

	void generateLookupTable() {
		assert(entries > 0 && "Hashlist is empty");

		for (int i = 0; i < cellCount; i++) {
			lookupTable[i] = ivec2(-1);
		}

		int currentStartIndex = 0;
		int previousCellHash = -1;
		for (int i = 0; i < entries - 1; i++) {
			if (hashList[i].y == hashList[i + 1].y)
				continue;

			lookupTable[hashList[i].y].x = currentStartIndex;

			if (previousCellHash != -1)
				lookupTable[previousCellHash].y = currentStartIndex - 1;
			
			previousCellHash = hashList[i].y;

			currentStartIndex = i + 1;
		}

		// In the case that somehow every position entry maps to the same cell hash
		if (previousCellHash != -1) lookupTable[previousCellHash].y = currentStartIndex - 1;
		
		lookupTable[hashList[entries - 1].y].x = currentStartIndex;
		lookupTable[hashList[entries - 1].y].y = entries - 1;
	}
};