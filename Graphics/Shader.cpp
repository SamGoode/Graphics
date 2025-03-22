#include "Shader.h"

#include <fstream>


bool Shader::init(const char* vertFileName, const char* fragFileName) {
	if (shader_id != 0) return false;

	unsigned int vs = loadShaderFromFile(GL_VERTEX_SHADER, vertFileName);
	glCompileShader(vs);

	unsigned int fs = loadShaderFromFile(GL_FRAGMENT_SHADER, fragFileName);
	glCompileShader(fs);

	shader_id = glCreateProgram();
	glAttachShader(shader_id, vs);
	glAttachShader(shader_id, fs);
	glBindAttribLocation(shader_id, 0, "Position");
	glBindAttribLocation(shader_id, 1, "Normal");
	glLinkProgram(shader_id);

	int success = GL_FALSE;
	glGetProgramiv(shader_id, GL_LINK_STATUS, &success);
	if (success == GL_FALSE) {
		int infoLogLength = 0;
		glGetProgramiv(shader_id, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength + 1];

		glGetProgramInfoLog(shader_id, infoLogLength, 0, infoLog);
		printf("Error: Failed to link Gizmo shader program!\n%s\n", infoLog);
		delete[] infoLog;

		return false;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	return true;
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
	unsigned int uniform = glGetUniformLocation(shader_id, name);
	//glUniform3fv(uniform, 1, glm::value_ptr(f));
	glUniform1f(uniform, f);
}

void Shader::bindUniform(const vec3& v3, const char* name) {
	unsigned int uniform = glGetUniformLocation(shader_id, name);
	glUniform3fv(uniform, 1, glm::value_ptr(v3));
}

void Shader::bindUniform(const mat4& m4, const char* name) {
	unsigned int uniform = glGetUniformLocation(shader_id, name);
	glUniformMatrix4fv(uniform, 1, false, glm::value_ptr(m4));
}