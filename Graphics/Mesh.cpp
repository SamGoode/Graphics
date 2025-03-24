#include "Mesh.h"

#include <assimp/scene.h>
#include <assimp/cimport.h>
#include <vector>


bool Mesh::init() {
	if (vao != 0) return false;

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * sizeof(vert), vertexBuffer, GL_STATIC_DRAW);
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(unsigned int), indexBuffer, GL_STATIC_DRAW);

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, maxInstances * sizeof(instanceData), instanceBuffer, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vert), 0); // position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vert), (void*)16); // normal

	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glEnableVertexAttribArray(7);
	glEnableVertexAttribArray(8);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)0); // transform
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)16); 
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)32);
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)48);
	glVertexAttribPointer(6, 3, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)64); // baseColor
	glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)76); // diffuseColor
	glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)88); // specColor
	glVertexAttribDivisor(2, 1);
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	glVertexAttribDivisor(8, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Mesh::loadFromFile(const char* name) {
	const aiScene* scene = aiImportFile(name, 0);

	aiMesh* mesh = scene->mMeshes[0];

	int faceCount = mesh->mNumFaces;
	std::vector<unsigned int> indices;
	for (int i = 0; i < faceCount; i++) {
		indices.push_back(mesh->mFaces[i].mIndices[0]);
		indices.push_back(mesh->mFaces[i].mIndices[2]);
		indices.push_back(mesh->mFaces[i].mIndices[1]);

		if (mesh->mFaces[i].mNumIndices == 4) {
			indices.push_back(mesh->mFaces[i].mIndices[0]);
			indices.push_back(mesh->mFaces[i].mIndices[3]);
			indices.push_back(mesh->mFaces[i].mIndices[2]);
		}
	}

	int vCount = mesh->mNumVertices;
	vertexBuffer = new vert[vCount];
	for (int i = 0; i < vCount; i++) {
		vertexBuffer[i].position = vec4(mesh->mVertices[i].x, mesh->mVertices[i].z, mesh->mVertices[i].y, 1);
		vertexBuffer[i].normal = vec4(mesh->mNormals[i].x, mesh->mNormals[i].z, mesh->mNormals[i].y, 0);
	}
	vertexCount += vCount;

	indexBuffer = new unsigned int[indices.size()];
	std::memcpy(&indexBuffer[indexCount], indices.data(), indices.size() * sizeof(unsigned int));
	indexCount += indices.size();
}

void Mesh::generateCube() {
	vertexBuffer = new vert[24];
	indexBuffer = new unsigned int[36];

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

	addQuad(vertices[4], vertices[5], vertices[6], vertices[7], vec3(0, 0, 1)); // Top
	addQuad(vertices[0], vertices[1], vertices[2], vertices[3], vec3(0, 0, -1)); // Bottom
	addQuad(vertices[1], vertices[0], vertices[4], vertices[5], vec3(1, 0, 0)); // Front
	addQuad(vertices[3], vertices[2], vertices[6], vertices[7], vec3(-1, 0, 0)); // Back
	addQuad(vertices[3], vertices[0], vertices[4], vertices[7], vec3(0, 1, 0)); // Left
	addQuad(vertices[2], vertices[1], vertices[5], vertices[6], vec3(0, -1, 0)); // Right
}

void Mesh::generateSphere() {
	vertexBuffer = new vert[84];
	indexBuffer = new unsigned int[360];

	vert verts[12 * 7];
	for (int i = 0; i < 12; i++) {
		float theta = i * (0.1667f * glm::pi<float>());
		float x = cos(theta);
		float y = sin(theta);

		// top and bottom most vertex are duplicated every arc so tex coords can be used later on
		for (int n = 0; n < 7; n++) {
			float beta = n * (0.1667f * glm::pi<float>());
			vec3 offsetFromCenter = vec3(x * sin(beta), y * sin(beta), cos(beta));
			verts[i * 7 + n].position = vec4(offsetFromCenter, 1);
			verts[i * 7 + n].normal = vec4(offsetFromCenter, 0);
		}
	}

	unsigned int count = 0;
	unsigned int sphereIndices[360];

	for (int i = 0; i < 84; i++) {
		int nextArc = (i + 7) % 84;

		if (i % 7 == 6) continue;

		if (i % 7 != 0) {
			sphereIndices[count++] = vertexCount + i;
			sphereIndices[count++] = vertexCount + nextArc;
			sphereIndices[count++] = vertexCount + i + 1;
		}

		if (i % 7 != 5) {
			sphereIndices[count++] = vertexCount + nextArc;
			sphereIndices[count++] = vertexCount + nextArc + 1;
			sphereIndices[count++] = vertexCount + i + 1;
		}
	}

	std::memcpy(&vertexBuffer[vertexCount], verts, 84 * sizeof(vert));
	vertexCount += 84;

	std::memcpy(&indexBuffer[indexCount], sphereIndices, 360 * sizeof(unsigned int));
	indexCount += 360;
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