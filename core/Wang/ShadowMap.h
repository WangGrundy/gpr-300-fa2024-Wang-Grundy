//glCreateFramebuffers(1, &shadowFBO);
//glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
//glGenTextures(1, &shadowMap);
//glBindTexture(GL_TEXTURE_2D, shadowMap);
////16 bit depth values, 2k resolution 
//glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, 2048, 2048);

#pragma once
#include <stdio.h>
#include <math.h>

#include "../../core/ew/external/glad.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>


namespace wang {
	struct asd {
		unsigned int fbo;
		unsigned int colorBuffer;
		unsigned int depthBuffer;
		unsigned int width;
		unsigned int height;
	};

	
}