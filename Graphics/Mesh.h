#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>

using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;


class Mesh {
private:
	struct vert {
		vec4 position;
		vec4 normal;
	};

	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ibo = 0;

	vert* vertexBuffer;
	int vertexCount = 0;

	unsigned int* indexBuffer;
	int indexCount = 0;

public:
	Mesh() {}
	~Mesh() {
		delete[] indexBuffer;
		glDeleteBuffers(1, &ibo);
		delete[] vertexBuffer;
		glDeleteBuffers(1, &vbo);
		glDeleteVertexArrays(1, &vao);
	}

	bool init();
	void draw() {
		glBindVertexArray(vao);
		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);
	}

	void loadFromFile(const char* name);

	void genCubeVerts();
	void genSphereVerts();
	void addQuad(vec4 v0, vec4 v1, vec4 v2, vec4 v3, vec3 faceNormal);
};