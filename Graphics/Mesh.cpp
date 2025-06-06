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

	delete[] vertexBuffer;
	delete[] indexBuffer;

	glGenBuffers(1, &instanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glBufferData(GL_ARRAY_BUFFER, MAX_INSTANCES * sizeof(instanceData), instanceBuffer, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vert), 0); // position
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(vert), (void*)12); // normal
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(vert), (void*)24); // texCoord

	glBindBuffer(GL_ARRAY_BUFFER, instanceVBO);
	glEnableVertexAttribArray(3);
	glEnableVertexAttribArray(4);
	glEnableVertexAttribArray(5);
	glEnableVertexAttribArray(6);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)0); // transform
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)16); 
	glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)32);
	glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)48);
	glEnableVertexAttribArray(7);
	glEnableVertexAttribArray(8);
	//glEnableVertexAttribArray(9);
	//glEnableVertexAttribArray(10);
	glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)64); // baseColor
	//glVertexAttribPointer(8, 3, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)76); // diffuseColor
	//glVertexAttribPointer(9, 3, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)88); // specularColor
	glVertexAttribPointer(8, 1, GL_FLOAT, GL_FALSE, sizeof(instanceData), (void*)76); // gloss
	glVertexAttribDivisor(3, 1);
	glVertexAttribDivisor(4, 1);
	glVertexAttribDivisor(5, 1);
	glVertexAttribDivisor(6, 1);
	glVertexAttribDivisor(7, 1);
	glVertexAttribDivisor(8, 1);
	//glVertexAttribDivisor(9, 1);
	//glVertexAttribDivisor(10, 1);

	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return true;
}

void Mesh::loadFromFile(const char* name) {
	assert(vertexBuffer == nullptr && indexBuffer == nullptr);

	const aiScene* scene = aiImportFile(name, 0);

	assert(scene != nullptr && "File does not exist");

	std::vector<unsigned int> indices;
	std::vector<vert> vertices;

	int meshCount = scene->mNumMeshes;

	// for now load all meshes in scene as one mesh
	for (int meshIndex = 0; meshIndex < meshCount; meshIndex++) {
		aiMesh* mesh = scene->mMeshes[meshIndex];

		int vertCount = mesh->mNumVertices;
		for (int i = 0; i < vertCount; i++) {
			vert vertex;
			vertex.position = vec3(mesh->mVertices[i].x, mesh->mVertices[i].z, mesh->mVertices[i].y);
			vertex.normal = vec3(mesh->mNormals[i].x, mesh->mNormals[i].z, mesh->mNormals[i].y);
			vertex.texCoord = mesh->HasTextureCoords(0) ? vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y) : vec2(0);
			vertices.push_back(vertex);
		}

		int faceCount = mesh->mNumFaces;
		for (int i = 0; i < faceCount; i++) {
			indices.push_back(vertexCount + mesh->mFaces[i].mIndices[0]);
			indices.push_back(vertexCount + mesh->mFaces[i].mIndices[2]);
			indices.push_back(vertexCount + mesh->mFaces[i].mIndices[1]);
			indexCount += 3;

			if (mesh->mFaces[i].mNumIndices == 4) {
				indices.push_back(vertexCount + mesh->mFaces[i].mIndices[0]);
				indices.push_back(vertexCount + mesh->mFaces[i].mIndices[3]);
				indices.push_back(vertexCount + mesh->mFaces[i].mIndices[2]);
				indexCount += 3;
			}
		}

		vertexCount += vertCount;
	}

	aiReleaseImport(scene);

	vertexBuffer = new vert[vertices.size()];
	std::memcpy(&vertexBuffer[0], vertices.data(), vertices.size() * sizeof(vert));

	indexBuffer = new unsigned int[indices.size()];
	std::memcpy(&indexBuffer[0], indices.data(), indices.size() * sizeof(unsigned int));
}

void Mesh::generatePlane() {
	assert(vertexBuffer == nullptr && indexBuffer == nullptr);

	vertexBuffer = new vert[4];
	indexBuffer = new unsigned int[6];

	addQuad(vec3(1, 1, 0), vec3(1, -1, 0), vec3(-1, -1, 0), vec3(-1, 1, 0), vec3(0, 0, 1));
}

void Mesh::generateCube() {
	assert(vertexBuffer == nullptr && indexBuffer == nullptr);

	vertexBuffer = new vert[24];
	indexBuffer = new unsigned int[36];

	vec3 vertices[8] = {
		vec3(0.5, 0.5, -0.5),
		vec3(0.5, -0.5, -0.5),
		vec3(-0.5, -0.5, -0.5),
		vec3(-0.5, 0.5, -0.5),
		vec3(0.5, 0.5, 0.5),
		vec3(0.5, -0.5, 0.5),
		vec3(-0.5, -0.5, 0.5),
		vec3(-0.5, 0.5, 0.5)
	};

	addQuad(vertices[4], vertices[5], vertices[6], vertices[7], vec3(0, 0, 1)); // Top
	addQuad(vertices[3], vertices[2], vertices[1], vertices[0], vec3(0, 0, -1)); // Bottom
	addQuad(vertices[5], vertices[4], vertices[0], vertices[1], vec3(1, 0, 0)); // Front
	addQuad(vertices[7], vertices[6], vertices[2], vertices[3], vec3(-1, 0, 0)); // Back
	addQuad(vertices[3], vertices[0], vertices[4], vertices[7], vec3(0, 1, 0)); // Left
	addQuad(vertices[6], vertices[5], vertices[1], vertices[2], vec3(0, -1, 0)); // Right
}

void Mesh::generateSphere() {
	assert(vertexBuffer == nullptr && indexBuffer == nullptr);

	vertexBuffer = new vert[91];
	indexBuffer = new unsigned int[360];

	constexpr float pi = glm::pi<float>();
	

	vert verts[13 * 7];
	for (int i = 0; i < 13; i++) {
		float theta = i * (0.1667f * pi);
		float x = cos(theta);
		float y = sin(theta);

		// top and bottom most vertex are duplicated every arc so tex coords can be used later on
		for (int n = 0; n < 7; n++) {
			float beta = n * (0.1667f * pi);
			vec3 offsetFromCenter = vec3(x * sin(beta), y * sin(beta), cos(beta));
			verts[i * 7 + n].position = offsetFromCenter;
			verts[i * 7 + n].normal = offsetFromCenter;
			verts[i * 7 + n].texCoord = vec2(i / 12.f, n / 6.f);
		}
	}

	unsigned int count = 0;
	unsigned int sphereIndices[360];

	for (int i = 0; i < 84; i++) {
		int nextArc = (i + 7);

		if (i % 7 == 6) continue;

		if (i % 7 != 5) {
			sphereIndices[count++] = vertexCount + i;
			sphereIndices[count++] = vertexCount + i + 1;
			sphereIndices[count++] = vertexCount + nextArc + 1;
		}

		if (i % 7 != 0) {
			sphereIndices[count++] = vertexCount + i;
			sphereIndices[count++] = vertexCount + nextArc + 1;
			sphereIndices[count++] = vertexCount + nextArc;
		}
	}

	std::memcpy(&vertexBuffer[vertexCount], verts, 91 * sizeof(vert));
	vertexCount += 91;

	std::memcpy(&indexBuffer[indexCount], sphereIndices, 360 * sizeof(unsigned int));
	indexCount += 360;
}

void Mesh::addQuad(vec3 v0, vec3 v1, vec3 v2, vec3 v3, vec3 faceNormal) {
	vert verts[4] = {
		vert{v0, faceNormal, vec2(0, 1)},
		vert{v1, faceNormal, vec2(1, 1)},
		vert{v2, faceNormal, vec2(1, 0)},
		vert{v3, faceNormal, vec2(0, 0)},
	};

	unsigned int quadIndices[6] = {
		vertexCount + 0,
		vertexCount + 2,
		vertexCount + 1,
		vertexCount + 0,
		vertexCount + 3,
		vertexCount + 2
	};

	std::memcpy(&vertexBuffer[vertexCount], verts, 4 * sizeof(vert));
	vertexCount += 4;

	std::memcpy(&indexBuffer[indexCount], quadIndices, 6 * sizeof(unsigned int));
	indexCount += 6;
}