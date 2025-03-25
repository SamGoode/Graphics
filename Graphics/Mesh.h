#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>

#include "MaterialProperties.h"

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;


class Mesh {
public:
	int textureID;

private:
	struct vert {
		vec4 position;
		vec4 normal;
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

	vert* vertexBuffer;
	int vertexCount = 0;

	unsigned int* indexBuffer;
	int indexCount = 0;

	instanceData instanceBuffer[100];
	int instanceCount = 0;
	const int maxInstances = 100;

public:
	Mesh() {}
	~Mesh() {
		glDeleteBuffers(1, &instanceVBO);
		delete[] indexBuffer;
		glDeleteBuffers(1, &ibo);
		delete[] vertexBuffer;
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

	void clearInstances() {
		instanceCount = 0;
	}
	void addInstance(mat4 instanceTransform, MaterialProperties material) {
		assert(instanceCount < maxInstances);
		instanceBuffer[instanceCount++] = { instanceTransform, material };
	}

	void loadFromFile(const char* name);

	// primitive shape meshes
	void generateCube();
	void generateSphere();

private:
	void addQuad(vec4 v0, vec4 v1, vec4 v2, vec4 v3, vec3 faceNormal);
};