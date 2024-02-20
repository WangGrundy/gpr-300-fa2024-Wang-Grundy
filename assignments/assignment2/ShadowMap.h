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