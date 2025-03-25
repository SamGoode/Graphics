#pragma once

#include "glmAddon.h"

class FrameBuffer {
private:
	unsigned int frameBuffer;
	unsigned int renderBuffer;

public:
	FrameBuffer() {}
	~FrameBuffer() {
		glDeleteFramebuffers(1, &frameBuffer);
		glDeleteRenderbuffers(1, &renderBuffer);
	}

	void init() {
		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);

		glGenRenderbuffers(1, &renderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1600, 900);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
			std::cout << "incomplete Framebuffer" << std::endl;
		}

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void bind() {
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	}

	void unbind() {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};