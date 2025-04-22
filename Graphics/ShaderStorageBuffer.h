#pragma once

// Data layout must follow std430 format
template<typename DataLayout>
class ShaderStorageBuffer {
private:
	unsigned int ssbo = 0;

public:
	DataLayout buffer;

public:
	ShaderStorageBuffer() {}
	~ShaderStorageBuffer() {
		glDeleteBuffers(1, &ssbo);
	}

	void init() {
		assert(ssbo == 0 && "Shader storage buffer already initialized");

		glGenBuffers(1, &ssbo);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(DataLayout), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void subData() {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(DataLayout), &buffer);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void subData(int offset, int size, const void* data) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void bind(GLuint bindingIndex) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, ssbo);
	}
};