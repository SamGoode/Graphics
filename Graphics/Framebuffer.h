#pragma once

#include "glmAddon.h"


class FrameBuffer {
private:
	unsigned int frameBuffer;
	unsigned int renderBuffer;
	GLenum rboAttachment;

	int textureCount = 0;
	const int textureMax = 12;
	RenderTexture textures[12];

	int width;
	int height;


public:
	FrameBuffer() {}
	~FrameBuffer() {
		glDeleteFramebuffers(1, &frameBuffer);
		glDeleteRenderbuffers(1, &renderBuffer);
	}

	void setSize(int _width, int _height) {
		width = _width;
		height = _height;
	}

	void init() {
		assert(frameBuffer == 0);

		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		GLenum* targets = new GLenum[textureCount];
		for (int i = 0; i < textureCount; i++) {
			targets[i] = GL_COLOR_ATTACHMENT0 + i;
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, textures[i].glID(), 0);
		}
		glDrawBuffers(textureCount, targets);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, rboAttachment, GL_RENDERBUFFER, renderBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Incomplete Framebuffer" << std::endl;


		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		delete[] targets;
	}

	// Uses render buffer of specified FrameBuffer instance
	void shareRenderBuffer(FrameBuffer& other) {
		renderBuffer = other.renderBuffer;
		rboAttachment = other.rboAttachment;
	}

	void genRenderBuffer(GLenum attachment, GLenum internalFormat) {
		assert(renderBuffer == 0);
		
		rboAttachment = attachment;

		glGenRenderbuffers(1, &renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void genRenderTexture(GLenum format) {
		assert(textureCount < textureMax);

		textures[textureCount].loadEmpty(format, width, height);
		textures[textureCount].init();
		textureCount++;
	}

	RenderTexture& getRenderTexture(int index) {
		assert(index < textureCount);

		return textures[index];
	}

	void bind() { glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer); }
	void unbind() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }

	void bindTexture(GLenum attachment, ITexture* texture) {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->glID(), 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};