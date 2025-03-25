#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>

#include "stb_image.h"

class Texture {
private:
	unsigned int texture = 0;

	int width;
	int height;
	int nrChannels;
	unsigned char* data = nullptr;

public:
	Texture() {}
	~Texture() {
		glDeleteTextures(1, &texture);
		stbi_image_free(data);
	}

	void init() {
		assert(data != nullptr);

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		GLenum format = GL_RGBA;
		if (nrChannels == 3) {
			format = GL_RGB;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		stbi_image_free(data);
	}

	void initEmpty(int _width, int _height, int _nrChannels) {
		assert(data == nullptr);

		width = _width;
		height = _height;
		nrChannels = _nrChannels;

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		GLenum format = GL_RGBA;
		if (nrChannels == 3) {
			format = GL_RGB;
		}

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void loadFromFile(const char* fileName) {
		assert(data == nullptr);
		//stbi_set_flip_vertically_on_load(true);
		data = stbi_load(fileName, &width, &height, &nrChannels, 0);
	}

	void generate1x1(int color) {
		assert(data == nullptr);

		width = 1;
		height = 1;
		nrChannels = 4;

		data = new unsigned char[4];
		data[0] = (unsigned char)(color >> 24);
		data[1] = (unsigned char)(color >> 16);
		data[2] = (unsigned char)(color >> 8);
		data[3] = (unsigned char)color;
	}

	void bind() {
		glBindTexture(GL_TEXTURE_2D, texture);
	}

	void bindToFramebuffer() {
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
	}
};