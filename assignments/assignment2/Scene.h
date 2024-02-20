#ifndef SCENE_H
#define SCENE_H

#pragma once

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <ew/external/glad.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>
#include <Wang/framebuffer.h>
#include <ew/procGen.h>

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
} material;

namespace wang {

	class Scene {
	public:
		Scene() {
			lightDirection = glm::vec3(0, 0, 0);
		}

		void CameraUpdate(GLFWwindow* window);
		void CameraSetUp();
		void LoadModelsAndTextures();
		void UpdateCurrentPostProcessingShader();
		void RenderInMain();

		ew::CameraController cameraController;
		ew::Transform monkeyTransform;

		//shaders
		ew::Shader depthonlyShader,
			litShader,
			invertShader,
			blurShader,
			noShader,
			gammaShader,
			grayscaleShader,
			coldShader;

		bool isColdShaderEnabled = false,
			isGrayscaleShaderEnabled = false,
			noPostProcessShader = false,
			isInvertShaderEnabled = false,
			isBlurShaderEnabled = false,
			isGammaShaderEnabled = false;

		//other variables for shaders
		bool postProcessShaderChanged = false;
		bool shaderChanged = false;
		float gammaVal = 0;

		//models
		ew::Model monkeyModel;

		//textures
		GLuint tileTexture;

		unsigned int dummyVAO;

		wang::Framebuffer newFrameBuffer;
		wang::Framebuffer shadowMapFrameBuffer;

		ew::Camera camera;
		ew::Camera camera2;
		ew::Camera* currentCamera;
		bool isCamera2 = false;

		glm::vec3 lightDirection;

		ew::Mesh planeMesh;

	private:
		//Global state
		int screenWidth = 1080;
		int screenHeight = 720;
		float prevFrameTime;
		float deltaTime;
	};
}
#endif // UICLASS_H