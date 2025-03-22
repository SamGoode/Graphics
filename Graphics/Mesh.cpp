#include "Mesh.h"



bool Mesh::init() {
	if (vao != 0) return false;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vert), vertexBuffer, GL_STATIC_DRAW);
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indexBuffer, GL_STATIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vert), 0); // position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vert), (void*)16); // normal

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::genCubeVerts() {
	vec4 vertices[8] = {
		vec4(0.5, 0.5, -0.5, 1),
		vec4(0.5, -0.5, -0.5, 1),
		vec4(-0.5, -0.5, -0.5, 1),
		vec4(-0.5, 0.5, -0.5, 1),
		vec4(0.5, 0.5, 0.5, 1),
		vec4(0.5, -0.5, 0.5, 1),
		vec4(-0.5, -0.5, 0.5, 1),
		vec4(-0.5, 0.5, 0.5, 1)
	};

	addQuad(vertices[0], vertices[1], vertices[2], vertices[3], vec3(0, 0, -1)); // Top
	addQuad(vertices[4], vertices[5], vertices[6], vertices[7], vec3(0, 0, 1)); // Bottom
	addQuad(vertices[1], vertices[0], vertices[4], vertices[5], vec3(1, 0, 0)); // Front
	addQuad(vertices[3], vertices[2], vertices[6], vertices[7], vec3(-1, 0, 0)); // Back
	addQuad(vertices[3], vertices[0], vertices[4], vertices[7], vec3(0, 1, 0)); // Left
	addQuad(vertices[2], vertices[1], vertices[5], vertices[6], vec3(0, -1, 0)); // Right
}

void Mesh::genSphereVerts() {
	vert verts[12 * 6];

	for (int i = 0; i < 12; i++) {
		float theta = i * (0.167f * glm::pi<float>());
		float x = cos(theta);
		float y = sin(theta);

		for (int n = 0; n < 6; n++) {
			float beta = n * (0.167f * glm::pi<float>());
			//points[i * 6 + n] = vec3(x * sin(beta), y * sin(beta), radius * cos(beta));
			vec3 offsetFromCenter = vec3(x * sin(beta), y * sin(beta), cos(beta));
			verts[i * 6 + n].position = vec4(offsetFromCenter, 1);
			verts[i * 6 + n].normal = vec4(offsetFromCenter, 0);
		}
	}

	unsigned int count = 0;
	unsigned int sphereIndices[324];

	for (int i = 0; i < 72; i++) {
		int nextArc = (i + 6) % 72;

		if (i % 6 == 5) {
			continue;
		}

		//addLine(center + points[i], center + points[i + 1], vec4(1));
		//addLine(verts[i].position, verts[i + 1].position, vec4(1));

		if (i % 6 != 0) {
			//addLine(center + points[i], center + points[nextArc], vec4(1));
			//addLine(verts[i].position, verts[nextArc].position, vec4(1));
			//addTri(center + points[i], center + points[nextArc], center + points[i + 1], color);
			sphereIndices[count++] = vertexCount + i;
			sphereIndices[count++] = vertexCount + nextArc;
			sphereIndices[count++] = vertexCount + i + 1;
		}

		//addTri(center + points[i + 1], center + points[nextArc], center + points[nextArc + 1], color);
		sphereIndices[count++] = vertexCount + i + 1;
		sphereIndices[count++] = vertexCount + nextArc;
		sphereIndices[count++] = vertexCount + nextArc + 1;
	}

	std::memcpy(&vertexBuffer[vertexCount], verts, 72 * sizeof(vert));
	vertexCount += 72;

	std::memcpy(&indexBuffer[indexCount], sphereIndices, 324 * sizeof(unsigned int));
	indexCount += 324;
}

void Mesh::addQuad(vec4 v0, vec4 v1, vec4 v2, vec4 v3, vec3 faceNormal) {
	vert verts[4] = {
		vert{v0, vec4(faceNormal, 0)},
		vert{v1, vec4(faceNormal, 0)},
		vert{v2, vec4(faceNormal, 0)},
		vert{v3, vec4(faceNormal, 0)},
	};

	unsigned int quadIndices[6] = {
		vertexCount + 0,
		vertexCount + 1,
		vertexCount + 2,
		vertexCount + 0,
		vertexCount + 2,
		vertexCount + 3
	};

	std::memcpy(&vertexBuffer[vertexCount], verts, 4 * sizeof(vert));
	vertexCount += 4;

	std::memcpy(&indexBuffer[indexCount], quadIndices, 6 * sizeof(unsigned int));
	indexCount += 6;
}