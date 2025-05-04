#include "Shader.h"

#include <fstream>
#include <string>
#include <iostream>

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
		printf("Error: Failed to link shader program!\n%s\n", infoLog);
		delete[] infoLog;
	}

	glDeleteShader(vs);
	glDeleteShader(fs);
}

unsigned int Shader::loadShaderFromFile(GLenum type, const char* fileName) {
	unsigned int shader = glCreateShader(type);

	std::ifstream fileStream;
	fileStream.open(fileName, std::ios::in | std::ios::binary);

	std::string fileString;
	
	std::string line;
	while (std::getline(fileStream, line)) {
		if (line[0] == '#') {
			if (line.substr(1, 7) == "include") {
				size_t nameStart = line.find_first_of('"', 7) + 1;
				size_t nameEnd = line.find_first_of('"', nameStart);
				std::string name = line.substr(nameStart, nameEnd - nameStart);

				std::ifstream includeFile;
				includeFile.open(name, std::ios::in | std::ios::binary);
				
				includeFile.seekg(0, includeFile.end);
				int fileLength = (int)includeFile.tellg();
				includeFile.seekg(0, includeFile.beg);

				char* buffer = new char[fileLength + 1];
				includeFile.read(buffer, fileLength);
				buffer[fileLength] = NULL;
				
				includeFile.close();

				fileString.append(buffer);
				fileString.append("\n");

				delete[] buffer;

				continue;
			}
		}
		fileString.append(line + "\n");
	}
	fileStream.close();

	//fileStream.read(test.data(), fileLength);
	//test.copy(fileText, sizeof(char) * (fileLength + 1));
	//fileString[fileLength] = NULL;

	const char* c_str = fileString.c_str();
	glShaderSource(shader, 1, &c_str, 0);
	//glShaderSource(shader, 1, (const char**)&fileText, 0);

	//delete[] fileText;

	return shader;
}


void Shader::bindUniform(const float& f, const char* name) {
	unsigned int uniformLocation = glGetUniformLocation(gl_id, name);
	glUniform1f(uniformLocation, f);
}

void Shader::bindUniform(const int& i, const char* name) {
	unsigned int uniformLocation = glGetUniformLocation(gl_id, name);
	glUniform1i(uniformLocation, i);
}

void Shader::bindUniform(const vec2& v2, const char* name) {
	unsigned int uniformLocation = glGetUniformLocation(gl_id, name);
	glUniform2fv(uniformLocation, 1, glm::value_ptr(v2));
}

void Shader::bindUniform(const vec3& v3, const char* name) {
	unsigned int uniformLocation = glGetUniformLocation(gl_id, name);
	glUniform3fv(uniformLocation, 1, glm::value_ptr(v3));
}

void Shader::bindUniform(const mat4& m4, const char* name) {
	unsigned int uniformLocation = glGetUniformLocation(gl_id, name);
	glUniformMatrix4fv(uniformLocation, 1, false, glm::value_ptr(m4));
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
		printf("Error: Failed to link shader program!\n%s\n", infoLog);
		delete[] infoLog;
	}

	glDeleteShader(cs);
}