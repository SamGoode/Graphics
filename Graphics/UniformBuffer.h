#pragma once

// Data layout must follow std140 format
template<typename DataLayout>
class UniformBuffer {
private:
	unsigned int ubo;

public:
	DataLayout buffer;

public:
	UniformBuffer() {}
	~UniformBuffer() {
		glDeleteBuffers(1, &ubo);
	}

	void init() {
		assert(ubo == 0 && "Uniform buffer already initialized");

		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(DataLayout), &buffer, GL_STATIC_DRAW);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void subData() {
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(DataLayout), &buffer);
		glBindBuffer(GL_UNIFORM_BUFFER, 0);
	}

	void bind(GLuint bindingIndex) {
		glBindBufferBase(GL_UNIFORM_BUFFER, bindingIndex, ubo);
	}
};