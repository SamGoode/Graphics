#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>

#include "MaterialProperties.h"

using glm::vec2;
using glm::vec3;
using glm::mat4;


#define MAX_INSTANCES 100

class Mesh {
public:
	int textureID = 0;

private:
	struct vert {
		vec3 position;
		vec3 normal;
		vec2 texCoord;
	};

	struct instanceData {
		mat4 transform;
		MaterialProperties material;
	};

	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ibo = 0;
	unsigned int instanceVBO = 0;

	unsigned int vertexCount = 0;
	vert* vertexBuffer = nullptr;

	unsigned int indexCount = 0;
	unsigned int* indexBuffer = nullptr;

	int instanceCount = 0;
	instanceData instanceBuffer[MAX_INSTANCES];

public:
	Mesh() {}
	~Mesh() {
		delete[] vertexBuffer;
		delete[] indexBuffer;
		glDeleteBuffers(1, &instanceVBO);
		glDeleteBuffers(1, &ibo);
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

	bool init();
	void renderInstances() {
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, instanceCount * sizeof(instanceData), instanceBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawElementsInstanced(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0, instanceCount);
	}

	void clearInstances() { instanceCount = 0; }
	void addInstance(mat4 instanceTransform, MaterialProperties material) { assert(instanceCount < MAX_INSTANCES); instanceBuffer[instanceCount++] = { instanceTransform, material }; }

	void loadFromFile(const char* name);

	// primitive shape meshes
	void generateCube();
	void generateSphere();
	void generatePlane();

private:
	void addQuad(vec3 v0, vec3 v1, vec3 v2, vec3 v3, vec3 faceNormal);
};