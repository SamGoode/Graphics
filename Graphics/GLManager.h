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


class GLwrapper {
private:
	struct vert {
		vec4 position;
		vec4 color;
		vec4 normal;
	};

	struct line {
		vert v0;
		vert v1;
	};

	unsigned int m_shader;

	unsigned int lineVBO;
	unsigned int lineVAO;

	line lines[2048];
	int lineCount = 0;
	const int lineMaxCount = 2048;


	unsigned int VAO;
	unsigned int VBO;
	unsigned int IBO;

	vert vertices[2048];
	int vertCount = 0;
	const int vertMaxCount = 2048;

	unsigned int indices[2048];
	int iboCount = 0;
	const int iboMaxCount = 2048;


public:
	GLwrapper();
	~GLwrapper();

	unsigned int loadShaderFromFile(GLenum type, const char* fileName);

	void draw(const mat4& projectionView, const vec3& directionalLight);

	void clear() { lineCount = 0; vertCount = 0; iboCount = 0; }

	void addLine(vec3 v0, vec3 v1, vec4 color);
	void addTri(vec3 v0, vec3 v1, vec3 v2, vec4 color);

	void addSphere(vec3 center, float radius, vec4 color);

	void addQuad(vec3 v0, vec3 v1, vec3 v2, vec3 v3, vec4 color, vec3 faceNormal);
	void addCuboid(vec3 center, vec3 extents, quat rotation, vec4 color);
};