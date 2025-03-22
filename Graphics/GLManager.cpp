#include "GLManager.h"

#include <fstream>


GLwrapper::GLwrapper() {
	unsigned int vs = loadShaderFromFile(GL_VERTEX_SHADER, "vert.glsl");
	glCompileShader(vs);

	unsigned int fs = loadShaderFromFile(GL_FRAGMENT_SHADER, "frag.glsl");
	glCompileShader(fs);

	m_shader = glCreateProgram();
	glAttachShader(m_shader, vs);
	glAttachShader(m_shader, fs);
	glBindAttribLocation(m_shader, 0, "Position");
	glBindAttribLocation(m_shader, 1, "Colour");
	glBindAttribLocation(m_shader, 2, "Normal");
	glLinkProgram(m_shader);

	int success = GL_FALSE;
	glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		int infoLogLength = 0;
		glGetProgramiv(m_shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength + 1];

		glGetProgramInfoLog(m_shader, infoLogLength, 0, infoLog);
		printf("Error: Failed to link Gizmo shader program!\n%s\n", infoLog);
		delete[] infoLog;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	glGenBuffers(1, &lineVBO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferData(GL_ARRAY_BUFFER, lineMaxCount * sizeof(line), lines, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &lineVAO);
	glBindVertexArray(lineVAO);
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vert), 0); // position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vert), (void*)16); // color
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vert), (void*)32); // normal

	// Index buffer
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertMaxCount * sizeof(vert), vertices, GL_DYNAMIC_DRAW);
	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, iboMaxCount * sizeof(unsigned int), indices, GL_DYNAMIC_DRAW);

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(vert), 0); // position
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(vert), (void*)16); // color
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(vert), (void*)32); // normal


	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLwrapper::~GLwrapper() {
	delete[] lines;
	glDeleteBuffers(1, &lineVBO);
	glDeleteVertexArrays(1, &lineVAO);

	delete[] vertices;
	delete[] indices;
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &IBO);
	glDeleteVertexArrays(1, &VAO);

	glDeleteProgram(m_shader);
}

unsigned int GLwrapper::loadShaderFromFile(GLenum type, const char* fileName) {
	unsigned int shader = glCreateShader(type);

	std::ifstream fileStream;
	fileStream.open(fileName, std::ios::in | std::ios::binary);

	fileStream.seekg(0, fileStream.end);
	int fileLength = fileStream.tellg();
	fileStream.seekg(0, fileStream.beg);

	char* fileText = new char[fileLength + 1];
	fileStream.read(fileText, fileLength);
	fileText[fileLength] = NULL;

	glShaderSource(shader, 1, (const char**)&fileText, 0);

	fileStream.close();
	delete[] fileText;

	return shader;
}

void GLwrapper::draw(const mat4& projectionView, const vec3& directionalLight) {
	int shader = 0;
	glGetIntegerv(GL_CURRENT_PROGRAM, &shader); // save current shader program id

	glUseProgram(m_shader);

	unsigned int projectionViewUniform = glGetUniformLocation(m_shader, "ProjectionView");
	glUniformMatrix4fv(projectionViewUniform, 1, false, glm::value_ptr(projectionView));

	unsigned int lightUniform = glGetUniformLocation(m_shader, "LightDirection");
	glUniform3fv(lightUniform, 1, glm::value_ptr(directionalLight));

	// Draw lines
	glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, lineCount * sizeof(line), lines);

	glBindVertexArray(lineVAO);
	glDrawArrays(GL_LINES, 0, lineCount * 2);


	// Index buffer test
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertCount * sizeof(vert), vertices);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, iboCount * sizeof(unsigned int), indices);

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, iboCount, GL_UNSIGNED_INT, 0);


	glUseProgram(shader); // switch back to previous shader program
}


void GLwrapper::addLine(vec3 v0, vec3 v1, vec4 color) {
	line l;
	l.v0.position = vec4(v0, 1);
	l.v1.position = vec4(v1, 1);
	l.v0.color = color;
	l.v1.color = color;

	lines[lineCount] = l;
	lineCount++;
}

void GLwrapper::addTri(vec3 v0, vec3 v1, vec3 v2, vec4 color) {
//	tri t;
//	t.v0.position = vec4(v0, 1);
//	t.v1.position = vec4(v1, 1);
//	t.v2.position = vec4(v2, 1);
//	t.v0.color = color;
//	t.v1.color = color;
//	t.v2.color = color;
//
//	tris[triCount] = t;
//	triCount++;
}

void GLwrapper::addSphere(vec3 center, float radius, vec4 color) {
	//vec3 points[12 * 6];
	vert verts[12 * 6];

	for (int i = 0; i < 12; i++) {
		float theta = i * (0.167f * glm::pi<float>());
		float x = radius * cos(theta);
		float y = radius * sin(theta);

		for (int n = 0; n < 6; n++) {
			float beta = n * (0.167f * glm::pi<float>());
			//points[i * 6 + n] = vec3(x * sin(beta), y * sin(beta), radius * cos(beta));
			vec3 offsetFromCenter = vec3(x * sin(beta), y * sin(beta), radius * cos(beta));
			verts[i * 6 + n].position = vec4(center + offsetFromCenter, 1);
			verts[i * 6 + n].normal = vec4(offsetFromCenter, 1);
			verts[i * 6 + n].color = color;
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
		addLine(verts[i].position, verts[i + 1].position, vec4(1));

		if (i % 6 != 0) {
			//addLine(center + points[i], center + points[nextArc], vec4(1));
			addLine(verts[i].position, verts[nextArc].position, vec4(1));
			//addTri(center + points[i], center + points[nextArc], center + points[i + 1], color);
			sphereIndices[count++] = vertCount + i;
			sphereIndices[count++] = vertCount + nextArc;
			sphereIndices[count++] = vertCount + i + 1;
		}

		//addTri(center + points[i + 1], center + points[nextArc], center + points[nextArc + 1], color);
		sphereIndices[count++] = vertCount + i + 1;
		sphereIndices[count++] = vertCount + nextArc;
		sphereIndices[count++] = vertCount + nextArc + 1;
	}

	std::memcpy(&vertices[vertCount], verts, 72 * sizeof(vert));
	vertCount += 72;

	std::memcpy(&indices[iboCount], sphereIndices, 324 * sizeof(unsigned int));
	iboCount += 324;
}

void GLwrapper::addQuad(vec3 v0, vec3 v1, vec3 v2, vec3 v3, vec4 color, vec3 faceNormal) {
	vert verts[4];
	verts[0].position = vec4(v0, 1);
	verts[1].position = vec4(v1, 1);
	verts[2].position = vec4(v2, 1);
	verts[3].position = vec4(v3, 1);

	for (int i = 0; i < 4; i++) {
		verts[i].color = color;
		verts[i].normal = vec4(normalize(faceNormal), 0);
	}

	unsigned int quadIndices[6] = {
		vertCount + 0,
		vertCount + 1,
		vertCount + 2,
		vertCount + 0,
		vertCount + 2,
		vertCount + 3
	};

	std::memcpy(&vertices[vertCount], verts, 4 * sizeof(vert));
	vertCount += 4;

	std::memcpy(&indices[iboCount], quadIndices, 6 * sizeof(unsigned int));
	iboCount += 6;
}

void GLwrapper::addCuboid(vec3 center, vec3 extents, quat rotation, vec4 color) {
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


	addQuad(verts[1], verts[0], verts[4], verts[5], color, -vX); // Back
	addQuad(verts[2], verts[3], verts[7], verts[6], color, vX); // Front
	addQuad(verts[0], verts[1], verts[2], verts[3], color, -vY); // Right
	addQuad(verts[4], verts[5], verts[6], verts[7], color, vY); // Left
	addQuad(verts[3], verts[0], verts[4], verts[7], color, -vZ); // Bottom
	addQuad(verts[2], verts[1], verts[5], verts[6], color, vZ); // Top


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
}

