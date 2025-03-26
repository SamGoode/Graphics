#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>

#include "Mesh.h"


using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;


class Shader {
private:
	unsigned int shader_id = 0;

public:
	Shader() {}
	~Shader() { glDeleteProgram(shader_id); }

	bool init(const char* vertFileName, const char* fragFileName);
	unsigned int loadShaderFromFile(GLenum type, const char* fileName);

	void use() { glUseProgram(shader_id); }
	void bindUniform(const float& f, const char* name);
	void bindUniform(const int& i, const char* name);
	void bindUniform(const vec3& v3, const char* name);
	void bindUniform(const mat4& m4, const char* name);
};