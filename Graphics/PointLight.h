#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>


using glm::vec3;
using glm::vec4;


#define MAX_POINTLIGHTS 128

class PointLight {
private:
	struct instanceData {
		vec3 lightPosition = vec3(0);
		float lightRadius = 0.f;
		vec3 lightColor = vec3(0);
	};

	unsigned int vao = 0;
	unsigned int instanceVBO = 0;

	unsigned int instanceCount = 0;
	instanceData instanceBuffer[MAX_POINTLIGHTS];

public:
	PointLight() {
		
	}
	~PointLight() {
		glDeleteBuffers(1, &instanceVBO);
		glDeleteVertexArrays(1, &vao);
	}

	void init() {
		assert(vao == 0 && "Pointlight manager already initialized");

		glGenBuffers(1, &instanceVBO);
		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, MAX_POINTLIGHTS * sizeof(instanceData), instanceBuffer, GL_DYNAMIC_DRAW);

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)0); // lightPosition
		glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)12); // lightRadius
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)16); // lightColor
		glVertexAttribDivisor(0, 1);
		glVertexAttribDivisor(1, 1);
		glVertexAttribDivisor(2, 1);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}

	void renderInstances() {
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, instanceCount * sizeof(instanceData), instanceBuffer);
		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDrawArraysInstanced(GL_TRIANGLES, 0, 36, instanceCount);
	}

	void clearInstances() {
		instanceCount = 0;
	}

	void addInstance(vec3 lightPosition, float lightRadius, vec3 lightColor) {
		assert(instanceCount < MAX_POINTLIGHTS);

		instanceBuffer[instanceCount++] = { lightPosition, lightRadius, lightColor };
	}
};