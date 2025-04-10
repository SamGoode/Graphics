#include "Shader.h"

#include <fstream>


void Shader::init(const char* vertFileName, const char* fragFileName) {
	assert(gl_id == 0 && "Shader already initialized");

	unsigned int vs = loadShaderFromFile(GL_VERTEX_SHADER, vertFileName);
	glCompileShader(vs);

	unsigned int fs = loadShaderFromFile(GL_FRAGMENT_SHADER, fragFileName);
	glCompileShader(fs);

	gl_id = glCreateProgram();
	glAttachShader(gl_id, vs);
	glAttachShader(gl_id, fs);
	glLinkProgram(gl_id);

	int success = GL_FALSE;
	glGetProgramiv(gl_id, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		int infoLogLength = 0;
		glGetProgramiv(gl_id, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength + 1];

		glGetProgramInfoLog(gl_id, infoLogLength, 0, infoLog);
		printf("Error: Failed to link Gizmo shader program!\n%s\n", infoLog);
		delete[] infoLog;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);
}

unsigned int Shader::loadShaderFromFile(GLenum type, const char* fileName) {
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


void Shader::bindUniform(const float& f, const char* name) {
	unsigned int uniform = glGetUniformLocation(gl_id, name);
	glUniform1f(uniform, f);
}

void Shader::bindUniform(const int& i, const char* name) {
	unsigned int uniform = glGetUniformLocation(gl_id, name);
	glUniform1i(uniform, i);
}

void Shader::bindUniform(const vec3& v3, const char* name) {
	unsigned int uniform = glGetUniformLocation(gl_id, name);
	glUniform3fv(uniform, 1, glm::value_ptr(v3));
}

void Shader::bindUniform(const mat4& m4, const char* name) {
	unsigned int uniform = glGetUniformLocation(gl_id, name);
	glUniformMatrix4fv(uniform, 1, false, glm::value_ptr(m4));
}

void Shader::bindUniformBuffer(GLuint bindingIndex, const char* name) {
	unsigned int uniformBlockIndex = glGetUniformBlockIndex(gl_id, name);
	glUniformBlockBinding(gl_id, uniformBlockIndex, bindingIndex);
}


void ComputeShader::init(const char* computeFileName, const char* empty) {
	assert(gl_id == 0 && "Shader already initialized");

	unsigned int cs = loadShaderFromFile(GL_COMPUTE_SHADER, computeFileName);
	glCompileShader(cs);

	gl_id = glCreateProgram();
	glAttachShader(gl_id, cs);
	glLinkProgram(gl_id);

	int success = GL_FALSE;
	glGetProgramiv(gl_id, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		int infoLogLength = 0;
		glGetProgramiv(gl_id, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength + 1];

		glGetProgramInfoLog(gl_id, infoLogLength, 0, infoLog);
		printf("Error: Failed to link Gizmo shader program!\n%s\n", infoLog);
		delete[] infoLog;
	}

	glDeleteShader(cs);
}