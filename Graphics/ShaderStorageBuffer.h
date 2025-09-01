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

	void clearNamedSubData(GLenum internalFormat, GLintptr offset, GLsizeiptr size, GLenum format, GLenum type, const void* data) {
		glClearNamedBufferSubData(ssbo, internalFormat, offset, size, format, type, data);
	}

	void getSubData(int offset, int size, const void* data) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
		glGetBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void bind(GLuint bindingIndex) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, ssbo);
	}
};


class DispatchIndirectBuffer {
private:
	unsigned int gl_id = 0;

public:
	DispatchIndirectBuffer() {}
	~DispatchIndirectBuffer() {
		glDeleteBuffers(1, &gl_id);
	}

	void init() {
		assert(gl_id == 0 && "Indirect Dispatch buffer already initialized");

		glGenBuffers(1, &gl_id);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_id);
		glBufferData(GL_SHADER_STORAGE_BUFFER, 3 * sizeof(unsigned int), NULL, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void clear() {
		unsigned int empty[3] = { 0, 0, 0 };

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, gl_id);
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, 3 * sizeof(unsigned int), empty);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void bindToIndex(GLuint bindingIndex) {
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bindingIndex, gl_id);
	}

	void bind() {
		glBindBuffer(GL_DISPATCH_INDIRECT_BUFFER, gl_id);
	}
};