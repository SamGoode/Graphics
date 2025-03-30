#pragma once

#include "glmAddon.h"


#define MAX_RENDER_TEXTURES 8

class FrameBuffer {
private:
	unsigned int frameBuffer;
	unsigned int renderBuffer;
	GLenum rboAttachment;

	int textureCount = 0;
	const int maxTextures = MAX_RENDER_TEXTURES;
	ITexture* textures[MAX_RENDER_TEXTURES];

	int width;
	int height;

public:
	FrameBuffer() {}
	~FrameBuffer() {
		for (int i = 0; i < textureCount; i++) {
			delete textures[i];
		}

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

		int colorAttachments = 0;
		GLenum targets[MAX_RENDER_TEXTURES];
		for (int i = 0; i < textureCount; i++) {
			GLenum attachment;

			switch (textures[i]->format) {
			case GL_DEPTH_COMPONENT:
			case GL_DEPTH_COMPONENT16:
			case GL_DEPTH_COMPONENT24:
			case GL_DEPTH_COMPONENT32:
				attachment = GL_DEPTH_ATTACHMENT;
				break;

			case GL_STENCIL_COMPONENTS:
				attachment = GL_STENCIL_ATTACHMENT;
				break;

			default:
				attachment = GL_COLOR_ATTACHMENT0 + colorAttachments;
				targets[colorAttachments] = attachment;
				colorAttachments++;
			}

			glFramebufferTexture(GL_FRAMEBUFFER, attachment, textures[i]->glID(), 0);
		}
		if (colorAttachments > 0) {
			glDrawBuffers(colorAttachments, targets);
		}
		else {
			glDrawBuffer(GL_NONE);
			glReadBuffer(GL_NONE);
		}

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, rboAttachment, GL_RENDERBUFFER, renderBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
			std::cout << "Incomplete Framebuffer" << std::endl;

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	// Uses render buffer of specified FrameBuffer instance
	void shareRenderBuffer(FrameBuffer& other) {
		renderBuffer = other.renderBuffer;
		rboAttachment = other.rboAttachment;
	}

	void sendStencilBuffer(unsigned int fboID) {
		glBindFramebuffer(GL_READ_FRAMEBUFFER, frameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fboID);
		glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_STENCIL_BUFFER_BIT, GL_NEAREST );
	}

	void genRenderBuffer(GLenum attachment, GLenum internalFormat) {
		assert(renderBuffer == 0);
		
		rboAttachment = attachment;

		glGenRenderbuffers(1, &renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, width, height);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);
	}

	void genTextureStorage(GLenum format) {
		assert(textureCount < maxTextures);

		textures[textureCount] = new TextureStorage();

		textures[textureCount]->loadEmpty(format, width, height);
		textures[textureCount]->init();
		textureCount++;
	}

	void genTextureImage(GLenum format) {
		assert(textureCount < maxTextures);

		textures[textureCount] = new Texture();

		textures[textureCount]->loadEmpty(format, width, height);
		textures[textureCount]->init();
		textureCount++;
	}

	ITexture* getRenderTexture(int index) {
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