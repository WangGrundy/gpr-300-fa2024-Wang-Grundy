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

	struct GBuffer {
		unsigned int fbo;
		unsigned int colorBuffers[3];
		unsigned int width;
		unsigned int height;
		unsigned int depthBuffer;
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

	//Creates a G-buffer with 3 color attachments
	GBuffer createGBuffer(unsigned int width, unsigned int height) {
		GBuffer gbuffer;
		gbuffer.width = width;
		gbuffer.height = height;

		glCreateFramebuffers(1, &gbuffer.fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, gbuffer.fbo);

		int formats[3] = {
			GL_RGB32F, //0 = World Position 
			GL_RGB16F, //1 = World Normal
			GL_RGB16F  //2 = Albedo
		};
		//Create 3 color textures
		for (size_t i = 0; i < 3; i++)
		{
			glGenTextures(1, &gbuffer.colorBuffers[i]);
			glBindTexture(GL_TEXTURE_2D, gbuffer.colorBuffers[i]);
			glTexStorage2D(GL_TEXTURE_2D, 1, formats[i], width, height);
			//Clamp to border so we don't wrap when sampling for post processing
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			//Attach each texture to a different slot.
		//GL_COLOR_ATTACHMENT0 + 1 = GL_COLOR_ATTACHMENT1, etc
			glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, gbuffer.colorBuffers[i], 0);
		}
		//Explicitly tell OpenGL which color attachments we will draw to
		const GLenum drawBuffers[3] = {
				GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2
		};
		glDrawBuffers(3, drawBuffers);

		// Add texture2D depth buffer
		glGenRenderbuffers(1, &gbuffer.depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, gbuffer.depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, gbuffer.depthBuffer);

		//TODO: Check for completeness
		GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
		if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
			printf("Framebuffer incomplete: %d", fboStatus);
		}

		//Clean up global state
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		return gbuffer;
	}

}
