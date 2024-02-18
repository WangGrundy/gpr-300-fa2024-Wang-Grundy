#pragma once
#include <stdio.h>
#include <math.h>

#include "../../core/ew/external/glad.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

namespace wang {
	struct Framebuffer {
		unsigned int fbo;
		unsigned int colorBuffer;
		unsigned int depthBuffer;
		unsigned int width;
		unsigned int height;
	};

	Framebuffer createFramebuffer(unsigned int width, unsigned int height, int colorFormat) {

		Framebuffer currentFrameBuffer;
		currentFrameBuffer.width = width;
		currentFrameBuffer.height = height;

		//Create Framebuffer Object
		glCreateFramebuffers(1, &currentFrameBuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, currentFrameBuffer.fbo);
		//Create 8 bit RGBA color buffer
		glGenTextures(1, &currentFrameBuffer.colorBuffer);
		glBindTexture(GL_TEXTURE_2D, currentFrameBuffer.colorBuffer);
		glTexStorage2D(GL_TEXTURE_2D, 1, colorFormat, width, height);
		//Attach color buffer to framebuffer
		glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, currentFrameBuffer.colorBuffer, 0);

		//create rbo with depth buffer and stencil buffer
		unsigned int rbo;
		glGenRenderbuffers(1, &rbo);
		glBindRenderbuffer(GL_RENDERBUFFER, rbo);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

		GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
			printf("Framebuffer incomplete: %d", fboStatus);
		}

		return currentFrameBuffer;
	}
}
