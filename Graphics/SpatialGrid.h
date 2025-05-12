#pragma once

#include <functional>
#include <algorithm>

#include <glm/ext.hpp>

using glm::ivec2;
using glm::ivec3;
using glm::uvec2;
using glm::uvec3;
using glm::vec3;
using glm::vec4;


// Uses an actual hashing algorithm to allow for infinite bounds
class SpatialHashGrid {
private:
	float cellSize = 0.f;

	unsigned int entries = 0;
	unsigned int capacity = 0;
	unsigned int cellCapacity = 0; // max entries per cell

	unsigned int* hashTable = nullptr;
	unsigned int* cellEntries = nullptr; // keeps track of amount of positions per used cell index
	unsigned int usedCells = 0;
	unsigned int* cells = nullptr;

	// 'hashTable' is sparse and maps to 'cells'
	// 'cells' is dense/contiguous and contains ids of positions within that cell.
	// 'cellEntries' is also dense/contiguous

public:
	SpatialHashGrid() {}
	~SpatialHashGrid() {
		delete[] hashTable;
		delete[] cellEntries;
		delete[] cells;
	}

	void init(float _cellSize, unsigned int _capacity, unsigned int _cellCapacity) {
		cellSize = _cellSize;

		capacity = _capacity;
		cellCapacity = _cellCapacity;

		hashTable = new unsigned int[capacity];
		cellEntries = new unsigned int[capacity] {0};
		cells = new unsigned int[capacity * cellCapacity] {0};

		resetHashTable();
	}

	
	void iterate3x3x3(ivec3 cellCoords, std::function<void(unsigned int)> iteratedFunc) {
		for (unsigned int i = 0; i < 27; i++) {
			ivec3 offset = ivec3(i % 3, (i / 3) % 3, i / 9) - ivec3(1);
			ivec3 offsetCellCoords = cellCoords + offset;

			unsigned int cellHash = getCellHash(offsetCellCoords);
			unsigned int cellIndex = hashTable[cellHash];

			if (cellIndex == 0xFFFFFFFF) continue;

			unsigned int entries = cellEntries[cellIndex];
			for (unsigned int n = 0; n < entries; n++) {
				unsigned int entryIndex = cells[cellIndex * cellCapacity + n];
				iteratedFunc(entryIndex);
			}
		}
	}

	const unsigned int* getHashTable() { return hashTable; }
	const unsigned int* getCellEntries() { return cellEntries; }
	unsigned int getUsedCells() { return usedCells; }
	const unsigned int* getCells() { return cells; }

	// returns grid coordinates of cell that 'position' falls into
	ivec3 getCellCoords(vec3 position) {
		return ivec3(floor(position / cellSize));
	}

	unsigned int getCellHash(const ivec3& cellCoords) {
		// Three large prime numbers (from the brain of Matthias Teschner)
		constexpr unsigned int p1 = 73856093;
		constexpr unsigned int p2 = 19349663; // Apparently this one isn't prime
		constexpr unsigned int p3 = 83492791;

		return ((p1 * (unsigned int)cellCoords.x) ^ (p2 * (unsigned int)cellCoords.y) ^ (p3 * (unsigned int)cellCoords.z)) % capacity;
	}

	void resetHashTable() {
		for (unsigned int i = 0; i < capacity; i++) {
			hashTable[i] = 0xFFFFFFFF;
		}
	}

	void clearCellEntries() {
		for (unsigned int i = 0; i < capacity; i++) {
			cellEntries[i] = 0;
		}
	}

	void clearData() {
		usedCells = 0;
		resetHashTable();
		clearCellEntries();
	}

	void generateHashTable(unsigned int count, const vec3* positions) {
		assert(count <= capacity);

		clearData();

		entries = count;
		for (unsigned int i = 0; i < entries; i++) {
			unsigned int cellHash = getCellHash(getCellCoords(positions[i]));
			//assert(cellEntries[cellHash] <= cellCapacity && "Cell's allocated capacity reached");

			if (hashTable[cellHash] == 0xFFFFFFFF) {
				hashTable[cellHash] = usedCells++; // Allocates new 'memory'
			}

			unsigned int cellIndex = hashTable[cellHash];
			cells[cellIndex * cellCapacity + cellEntries[cellIndex]] = i;
			cellEntries[cellIndex]++;
		}
	}

	void generateHashTable(unsigned int count, const vec4* positions) {
		assert(count <= capacity);

		// these need to be reset
		usedCells = 0;
		resetHashTable();
		clearCellEntries();

		entries = count;
		for (unsigned int i = 0; i < entries; i++) {
			unsigned int cellHash = getCellHash(getCellCoords(vec3(positions[i])));
			//assert(cellEntries[cellHash] <= cellCapacity && "Cell's allocated capacity reached");

			if (hashTable[cellHash] == 0xFFFFFFFF) {
				hashTable[cellHash] = usedCells++; // Allocates new 'memory'
			}

			unsigned int cellIndex = hashTable[cellHash];
			cells[cellIndex * cellCapacity + cellEntries[cellIndex]] = i;
			cellEntries[cellIndex]++;
		}
	}
};

class SpatialGrid {
private:
	vec3 bounds = vec3(0);
	float cellSize = 0.f;

	ivec3 gridBounds = ivec3(0);
	unsigned int cellCount = 0;

	unsigned int entries = 0;
	unsigned int capacity = 0;

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

		for (unsigned int i = 0; i < cellCount; i++) {
			lookupTable[i] = ivec2(-1);
		}
	}

	ivec3 getGridBounds() { return gridBounds; }
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
		for (unsigned int i = 0; i < entries; i++) {
			hashList[i] = ivec2(i, getCellHash(getCellCoords(positions[i])));
		}
	}

	void sortHashList() { std::sort(&hashList[0], &hashList[entries - 1] + 1, [](const ivec2& a, const ivec2& b) {return a.y < b.y;}); }

	void generateLookupTable() {
		assert(entries > 0 && "Hashlist is empty");

		for (unsigned int i = 0; i < cellCount; i++) {
			lookupTable[i] = ivec2(-1);
		}

		int currentStartIndex = 0;
		int previousCellHash = -1;
		for (unsigned int i = 0; i < entries - 1; i++) {
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