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
}material;

ew::CameraController cameraController;
ew::Transform monkeyTransform;
ew::Camera camera;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void RenderInMain();
void CameraUpdate(GLFWwindow* window);
void CameraSetUp();
void LoadModelsAndTextures();
void UpdateCurrentPostProcessingShader();
void DrawScene();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Shader litShader, invertShader, blurShader, noShader, gammaShader, grayscaleShader, coldShader;
ew::Shader geometryShader, lightingShader;
bool shaderChanged = false;
ew::Model monkeyModel;

unsigned int dummyVAO;

wang::Framebuffer newFrameBuffer;
GLuint tileTexture;

float gammaVal = 0;

bool isColdShaderEnabled = false, isGrayscaleShaderEnabled = false, noPostProcessShader = false, isInvertShaderEnabled = false, isBlurShaderEnabled = false, isGammaShaderEnabled = false;
bool postProcessShaderChanged = false;

ew::Mesh planeMesh;
wang::GBuffer gBuffer;

int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	planeMesh = ew::Mesh(ew::createPlane(10, 10, 10));

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	LoadModelsAndTextures();
	CameraSetUp();

	newFrameBuffer = wang::createFramebuffer(screenWidth, screenHeight, GL_RGB32F);
	glCreateVertexArrays(1, &dummyVAO);

	gBuffer = wang::createGBuffer(screenWidth, screenHeight);

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		CameraUpdate(window);
		RenderInMain();
		drawUI();

		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}

void RenderInMain() {
	
	//Rotate model around Y axis
	monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

	///Geometry Pass
	///We must next create a special shader program that will be used to draw surface attributes to all of our textures.
	///This will be used in a first pass, called the geometry pass, before shading later.	
	//in render loop before shading
	//RENDER SCENE TO G-BUFFER
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer.fbo);
	glViewport(0, 0, gBuffer.width, gBuffer.height);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	geometryShader.use();
	geometryShader.setMat4("_Model", glm::mat4(1.0f));
	geometryShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
	geometryShader.setMat4("_Model", monkeyTransform.modelMatrix());
	glBindTextureUnit(0, tileTexture);

	DrawScene();

	//LIGHTING PASS
	//if using post processing, we draw to our offscreen framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, newFrameBuffer.fbo);
	glViewport(0, 0, newFrameBuffer.width, newFrameBuffer.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	lightingShader.use();
	//TODO: Set the rest of your lighting uniforms for deferredShader. (same way we did this for lit.frag)
	lightingShader.setVec3("_EyePos", camera.position);
	lightingShader.setFloat("_Material.Ka", material.Ka);
	lightingShader.setFloat("_Material.Kd", material.Kd);
	lightingShader.setFloat("_Material.Ks", material.Ks);
	lightingShader.setFloat("_Material.Shininess", material.Shininess);

	//Bind g-buffer textures
	glBindTextureUnit(0, gBuffer.colorBuffers[0]);
	glBindTextureUnit(1, gBuffer.colorBuffers[1]);
	glBindTextureUnit(2, gBuffer.colorBuffers[2]);

	glBindVertexArray(dummyVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	///Now that we have completed our geometry pass, we can move onto lighting.
	///You can think of deferred shading as a fancy post process effect.
	///We will create a fullscreen triangle, and sample our G-buffers and lighting uniforms
	///to calculate the final color for each pixel.

	glBindTextureUnit(0, tileTexture);

	 // Draw to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, screenWidth, screenHeight);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the current post-processing shader
	UpdateCurrentPostProcessingShader();
	
    // Bind the color buffer texture
    glBindTextureUnit(0, newFrameBuffer.colorBuffer);

    // Draw the quad
    glBindVertexArray(dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6); // 6 for quad, 3 for triangle
}

void DrawScene() {
	monkeyModel.draw(); //Draws monkey model using current shader
}

void LoadModelsAndTextures() {
	//Handles to OpenGL object are unsigned integers
	tileTexture = ew::loadTexture("assets/materials/Tile/Tiles130.jpg");

	//load shader
	litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	invertShader = ew::Shader("assets/quad.vert", "assets/invert.frag");
	blurShader = ew::Shader("assets/quad.vert", "assets/blur.frag");
	noShader = ew::Shader("assets/quad.vert", "assets/none.frag");
	gammaShader = ew::Shader("assets/quad.vert", "assets/gamma.frag");
	grayscaleShader = ew::Shader("assets/quad.vert", "assets/grayscale.frag");
	coldShader = ew::Shader("assets/quad.vert", "assets/cold.frag");

	geometryShader = ew::Shader("assets/geometryPass.vert", "assets/geometryPass.frag");
	lightingShader = ew::Shader("assets/quad.vert", "assets/defferendLit.frag");

	//load model
	monkeyModel = ew::Model("assets/suzanne.obj");

	//Bind texture to default in shader 0
	glBindTextureUnit(0, tileTexture);
	//Make "_MainTex" sampler2D sample from the 2D texture bound to unit 0
	litShader.use();
	litShader.setInt("_MainTex", 0);
	litShader.setInt("_MainText_Normal", 1);

	lightingShader.use();
	lightingShader.setInt("_MainTex", 0);
	lightingShader.setInt("_MainText_Normal", 1);
}

void UpdateCurrentPostProcessingShader() {

	noShader.use();

	postProcessShaderChanged = true;

	if (isInvertShaderEnabled) {
		invertShader.use();
	}
	if (isBlurShaderEnabled) {
		blurShader.use();
	}
	if (isGammaShaderEnabled) {
		gammaShader.use();
	}
	if (isGrayscaleShaderEnabled) {
		//glEnable(GL_FRAMEBUFFER_SRGB);
		grayscaleShader.use();
	}
	else {
		glDisable(GL_FRAMEBUFFER_SRGB);
	}
	if (isColdShaderEnabled) {
		coldShader.use();
	}

	if (!postProcessShaderChanged) {
		noShader.use();
	}

	postProcessShaderChanged = false;
}

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::SetNextWindowSize(ImVec2(300, 500));
	ImGui::Begin("Settings");
	ImGui::Text("Add Controls Here!");

	if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}

	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}

	if (ImGui::CollapsingHeader("Post Processing Effects")) {
		ImGui::Checkbox("none", &noPostProcessShader);
		ImGui::Checkbox("Invert", &isInvertShaderEnabled);
		ImGui::Checkbox("Blur", &isBlurShaderEnabled);
		ImGui::Checkbox("Gamma", &isGammaShaderEnabled);
		ImGui::SameLine();
		if (ImGui::DragFloat("Gamma value", &gammaVal, 0.05, 0, 10)) {
			gammaShader.setFloat("gammaValue", gammaVal);

		}
		ImGui::Checkbox("grayscale", &isGrayscaleShaderEnabled);
		ImGui::Checkbox("coldness", &isColdShaderEnabled);

		postProcessShaderChanged = true;
	}
	ImGui::End();

	ImGui::Begin("GBuffers"); {
		ImVec2 texSize = ImVec2(gBuffer.width / 4, gBuffer.height / 4);
		for (size_t i = 0; i < 3; i++)
		{
			ImGui::Image((ImTextureID)gBuffer.colorBuffers[i], texSize, ImVec2(0, 1), ImVec2(1, 0));
		}
		ImGui::End();
	}

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CameraSetUp() {
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees
}

void CameraUpdate(GLFWwindow* window) {
	cameraController.move(window, &camera, deltaTime);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}