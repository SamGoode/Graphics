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

	unsigned int vao;
	unsigned int vbo;
	unsigned int ibo;
	unsigned int instanceVBO;

	int vertexCount = 0;
	vert* vertexBuffer;

	int indexCount = 0;
	unsigned int* indexBuffer;

	int instanceCount = 0;
	const int maxInstances = 100;
	instanceData instanceBuffer[100];

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
	void generatePlane();

private:
	void addQuad(vec4 v0, vec4 v1, vec4 v2, vec4 v3, vec3 faceNormal);
};