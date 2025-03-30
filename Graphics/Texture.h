#pragma once

#include "glad.h"
#include <GLFW/glfw3.h>

#include "stb_image.h"


struct ITexture {
protected:
	unsigned int gl_id = 0;

public:
	GLenum format;
	int width;
	int height;
	unsigned char* data = nullptr;

	virtual ~ITexture() {
		glDeleteTextures(1, &gl_id);
		if(data) stbi_image_free(data);
	}

	virtual void init() = 0;

	void loadEmpty(GLenum _format, int _width, int _height) {
		format = _format;
		height = _height;
		width = _width;
	}

	unsigned int glID() { return gl_id; }

	void bind(GLenum textureUnit = GL_TEXTURE0) { 
		glActiveTexture(textureUnit);
		glBindTexture(GL_TEXTURE_2D, gl_id); 
	}
};


class TextureStorage : public ITexture {
public:
	TextureStorage() {}

	virtual void init() override {
		assert(data == nullptr && gl_id == 0);

		glGenTextures(1, &gl_id);
		glBindTexture(GL_TEXTURE_2D, gl_id);

		glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glBindTexture(GL_TEXTURE_2D, 0);
	}
};


class Texture : public ITexture {
public:
	Texture() {}

	virtual void init() override {
		assert(gl_id == 0);

		glGenTextures(1, &gl_id);
		glBindTexture(GL_TEXTURE_2D, gl_id);

		if(data)
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, NULL);

		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		float color[4] = {1.f, 0.f, 0.f, 0.f};
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		stbi_image_free(data);
		data = nullptr;
	}

	void loadFromFile(const char* fileName, bool shouldFlip = false) {
		assert(gl_id == 0);

		stbi_set_flip_vertically_on_load(shouldFlip);

		int nrChannels;
		data = stbi_load(fileName, &width, &height, &nrChannels, 0);

		format = GL_RGBA;
		if (nrChannels == 3) format = GL_RGB;
	}

	void generate1x1(int color) {
		assert(gl_id == 0);

		width = 1;
		height = 1;
		format = GL_RGBA;

		data = new unsigned char[4];
		data[0] = (unsigned char)(color >> 24);
		data[1] = (unsigned char)(color >> 16);
		data[2] = (unsigned char)(color >> 8);
		data[3] = (unsigned char)color;
	}
};