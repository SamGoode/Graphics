#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/fwd.hpp>

#include "Mesh.h"

#include <string>


using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat4;
using glm::quat;


class Shader {
protected:
	unsigned int gl_id = 0;

	std::string path = "shaders/";

public:
	Shader() {}
	virtual ~Shader() { glDeleteProgram(gl_id); }

	virtual void init(const char* vertFileName, const char* fragFileName);

	void use() { glUseProgram(gl_id); }
	void bindUniform(const float& f, const char* name);
	void bindUniform(const int& i, const char* name);
	void bindUniform(const vec2& v2, const char* name);
	void bindUniform(const vec3& v3, const char* name);
	void bindUniform(const mat4& m4, const char* name);
	void bindUniformBuffer(GLuint bindingIndex, const char* name);

protected:
	unsigned int loadShaderFromFile(GLenum type, const char* fileName);
};

class ComputeShader : public Shader {
public:
	ComputeShader() {}

	virtual void init(const char* computeFileName, const char* empty = NULL) override;
};