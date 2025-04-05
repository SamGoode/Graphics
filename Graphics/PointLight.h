#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>


using glm::vec3;
using glm::vec4;


class PointLight {
private:
	struct instanceData {
		vec3 lightPosition;
		float lightRadius;
		vec3 lightColor;
	};

	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ibo = 0;
	unsigned int instanceVBO = 0;

	unsigned int instanceCount = 0;
	const int maxInstances = 100;
	instanceData instanceBuffer[100];


public:
	PointLight() {}
	~PointLight() {
		glDeleteBuffers(1, &instanceVBO);
		glDeleteBuffers(1, &ibo);
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

	bool init() {
		if (vao != 0) return false;

		vec3 vertices[8] = {
			vec3(1, 1, -1),
			vec3(1, -1, -1),
			vec3(-1, -1, -1),
			vec3(-1, 1, -1),
			vec3(1, 1, 1),
			vec3(1, -1, 1),
			vec3(-1, -1, 1),
			vec3(-1, 1, 1)
		};

		unsigned int indices[36] = {
			4, 6, 5, 4, 7, 6,
			3, 1, 2, 3, 0, 1,
			5, 0, 4, 5, 1, 0,
			3, 2, 6, 3, 6, 7,
			3, 4, 0, 3, 7, 4,
			2, 1, 5, 2, 5, 6
		};

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(vec3), vertices, GL_STATIC_DRAW);
		glGenBuffers(1, &ibo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 36 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

		glGenBuffers(1, &instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, maxInstances * sizeof(instanceData), instanceBuffer, GL_DYNAMIC_DRAW);

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3), 0); // position

		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)0); // lightPosition
		glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)12); // lightRadius
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)16); // lightRadius
		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);
		glVertexAttribDivisor(3, 1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void renderInstances() {
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, instanceCount * sizeof(instanceData), instanceBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0, instanceCount);
	}

	void clearInstances() {
		instanceCount = 0;
	}

	void addInstance(vec3 lightPosition, float lightRadius, vec3 lightColor) {
		assert(instanceCount < maxInstances);

		instanceBuffer[instanceCount++] = { lightPosition, lightRadius, lightColor };
	}
};