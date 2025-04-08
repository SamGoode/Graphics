#pragma once

#include <algorithm>

#include <glm/ext.hpp>

using glm::uvec2;
using glm::uvec3;
using glm::vec3;


class SpatialGrid {
private:
	vec3 bounds;
	float cellSize;

	uvec3 gridBounds;
	unsigned int cellCount;

	unsigned int entries = 0;
	unsigned int capacity;

	// (position index, cell hash)
	uvec2* hashList = nullptr;
	// (start index, end index)
	uvec2* lookupTable = nullptr;


public:
	SpatialGrid() {}
	~SpatialGrid() {
		delete[] hashList;
		delete[] lookupTable;
	}

	void init(vec3 _bounds, float _cellSize, unsigned int _capacity) {
		bounds = _bounds;
		cellSize = _cellSize;

		gridBounds = uvec3(ceil(_bounds / _cellSize));
		cellCount = gridBounds.x * gridBounds.y * gridBounds.z;

		capacity = _capacity;
		hashList = new uvec2[_capacity];
		lookupTable = new uvec2[cellCount];
	}

	const uvec2* getHashList() { return hashList; }
	const uvec2* getLookupTable() { return lookupTable; }

	// returns grid coordinates of cell that 'position' falls into
	uvec3 getCellCoords(vec3 position) { return uvec3(floor(glm::clamp(position, vec3(0), bounds) / cellSize)); }
	bool isValidCoords(uvec3 cellCoords) { return cellCoords.x < gridBounds.x && cellCoords.y < gridBounds.y && cellCoords.z < gridBounds.z; }

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
			hashList[i] = uvec2(i, getCellHash(getCellCoords(positions[i])));
		}
	}

	void sortHashList() { std::sort(hashList, &hashList[entries - 1] + 1, [](const uvec2& a, const uvec2& b) {return a.y < b.y;}); }

	void generateLookupTable() {
		assert(entries > 0 && "Hashlist is empty");

		for (int i = 0; i < cellCount; i++) {
			lookupTable[i] = uvec2(-1, -1);
		}

		int currentStartIndex = 0;
		int previousCellHash = -1;
		for (int i = 0; i < entries - 1; i++) {
			if (hashList[i].y == hashList[i + 1].y) continue;

			lookupTable[hashList[i].y].x = currentStartIndex;

			if (previousCellHash != -1) lookupTable[previousCellHash].y = currentStartIndex - 1;
			previousCellHash = hashList[i].y;

			currentStartIndex = i + 1;
		}

		// In the case that somehow every position entry maps to the same cell hash
		if (previousCellHash != -1) lookupTable[previousCellHash].y = currentStartIndex - 1;
		
		lookupTable[hashList[entries - 1].y].x = currentStartIndex;
		lookupTable[hashList[entries - 1].y].y = entries - 1;
	}
};