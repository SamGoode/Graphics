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

	struct tri {
		vert v0;
		vert v1;
		vert v2;
	};

	unsigned int m_shader;

	unsigned int lineVBO;
	unsigned int lineVAO;

	unsigned int triVBO;
	unsigned int triVAO;

	line lines[2048];
	int lineCount = 0;
	const int lineMaxCount = 2048;

	tri tris[2048];
	int triCount = 0;
	const int triMaxCount = 2048;

public:
	GLwrapper();
	~GLwrapper();

	void clear() {
		lineCount = 0;
		triCount = 0;
	}

	void addLine(vec3 v0, vec3 v1, vec4 color) {
		line l;
		l.v0.position = vec4(v0, 1);
		l.v1.position = vec4(v1, 1);
		l.v0.color = color;
		l.v1.color = color;

		lines[lineCount] = l;
		lineCount++;
	}

	void addTriangle(vec3 v0, vec3 v1, vec3 v2, vec4 color) {
		tri t;
		t.v0.position = vec4(v0, 1);
		t.v1.position = vec4(v1, 1);
		t.v2.position = vec4(v2, 1);
		t.v0.color = color;
		t.v1.color = color;
		t.v2.color = color;

		tris[triCount] = t;
		triCount++;
	}

	void addSphere(vec3 center, float radius, vec4 color) {

		vec3 v4Array[110];

		for (int i = 0; i <= 10; i++) {
			float theta = (i / 10) * 2 * glm::pi<float>();
			float y = radius * sin(theta);
			float z = radius * cos(theta);

			for (int n = 0; n <= 10; n++) {
				float beta = (n / 10) * 2 * glm::pi<float>();
				vec3 point = vec3(-z * sin(beta), y, -z * cos(theta));
				float invRadius = 1 / radius;
				vec3 norm = point * invRadius;

				v4Array[i * 10 + (n % 10)] = point;
			}			
		}

		for (int i = 0; i < 100; i++) {
			addLine(center + v4Array[i], center + v4Array[i + 10], vec4(1));

			//addTriangle(center + v4Array[i + 11], center + v4Array[i + 10], v4Array[i + ])
		}
	}

	void addCuboid(vec3 center, vec3 extents, quat rotation, vec4 color) {
		vec3 verts[8];

		// rotated axis-aligned extents
		vec3 vX = rotation * vec3(extents.x, 0, 0);
		vec3 vY = rotation * vec3(0, extents.y, 0);
		vec3 vZ = rotation * vec3(0, 0, extents.z);

		// top verts
		verts[0] = center - vX - vZ - vY;
		verts[1] = center - vX + vZ - vY;
		verts[2] = center + vX + vZ - vY;
		verts[3] = center + vX - vZ - vY;

		// bottom verts
		verts[4] = center - vX - vZ + vY;
		verts[5] = center - vX + vZ + vY;
		verts[6] = center + vX + vZ + vY;
		verts[7] = center + vX - vZ + vY;


		addLine(verts[0], verts[1], vec4(1));
		addLine(verts[1], verts[2], vec4(1));
		addLine(verts[2], verts[3], vec4(1));
		addLine(verts[3], verts[0], vec4(1));

		addLine(verts[4], verts[5], vec4(1));
		addLine(verts[5], verts[6], vec4(1));
		addLine(verts[6], verts[7], vec4(1));
		addLine(verts[7], verts[4], vec4(1));

		addLine(verts[0], verts[4], vec4(1));
		addLine(verts[1], verts[5], vec4(1));
		addLine(verts[2], verts[6], vec4(1));
		addLine(verts[3], verts[7], vec4(1));

		// top
		addTriangle(verts[2], verts[1], verts[0], color);
		addTriangle(verts[3], verts[2], verts[0], color);

		// bottom
		addTriangle(verts[5], verts[6], verts[4], color);
		addTriangle(verts[6], verts[7], verts[4], color);

		// front
		addTriangle(verts[4], verts[3], verts[0], color);
		addTriangle(verts[7], verts[3], verts[4], color);

		// back
		addTriangle(verts[1], verts[2], verts[5], color);
		addTriangle(verts[2], verts[6], verts[5], color);

		// left
		addTriangle(verts[0], verts[1], verts[4], color);
		addTriangle(verts[1], verts[5], verts[4], color);

		// right
		addTriangle(verts[2], verts[3], verts[7], color);
		addTriangle(verts[6], verts[2], verts[7], color);
	}

	void draw(const glm::mat4& projectionView) {
		int shader = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &shader); // save current shader program id

		glUseProgram(m_shader);

		unsigned int projectionViewUniform = glGetUniformLocation(m_shader, "ProjectionView");
		glUniformMatrix4fv(projectionViewUniform, 1, false, glm::value_ptr(projectionView));

		// Draw lines
		glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, lineCount * sizeof(line), lines);

		glBindVertexArray(lineVAO);
		glDrawArrays(GL_LINES, 0, lineCount * 2);

		// Draw triangles
		glBindBuffer(GL_ARRAY_BUFFER, triVBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, triCount * sizeof(tri), tris);

		glBindVertexArray(triVAO);
		glDrawArrays(GL_TRIANGLES, 0, triCount * 3);

		glUseProgram(shader); // switch back to old shader program
	}
};