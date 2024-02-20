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
};

Material material;

ew::CameraController cameraController;
ew::Transform monkeyTransform;

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);
void drawUI();
void RenderInMain();
void CameraUpdate(GLFWwindow* window);
void CameraSetUp();
void LoadModelsAndTextures();
void UpdateCurrentPostProcessingShader();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

int HDR = 0;

ew::Shader depthonlyShader, litShader, invertShader, blurShader, noShader, gammaShader, grayscaleShader, coldShader;
bool shaderChanged = false;
ew::Model monkeyModel;

unsigned int dummyVAO;

wang::Framebuffer newFrameBuffer;
GLuint tileTexture;

float gammaVal = 0;

bool isColdShaderEnabled = false, isGrayscaleShaderEnabled = false, noPostProcessShader = false, isInvertShaderEnabled = false, isBlurShaderEnabled = false, isGammaShaderEnabled = false;
bool postProcessShaderChanged = false;

wang::Framebuffer shadowMapFrameBuffer;
ew::Camera camera;
ew::Camera camera2;
glm::vec3 lightDirection(0,0,0);

ew::Camera* currentCamera;

bool isCamera2 = false;
ew::Mesh planeMesh;

int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	ew::Camera* currentCamera = &camera;
	planeMesh = ew::Mesh(ew::createPlane(10, 10, 10));

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	LoadModelsAndTextures();
	CameraSetUp();	

	newFrameBuffer = wang::createFramebuffer(screenWidth, screenHeight, GL_RGB32F);

	shadowMapFrameBuffer = wang::createFramebuffer(screenWidth, screenHeight, GL_RGB32F);

	//create a shadow map
	glCreateFramebuffers(1, &shadowMapFrameBuffer.fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFrameBuffer.fbo);
	//not drawing colors.
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glGenTextures(1, &shadowMapFrameBuffer.depthBuffer);
	glBindTexture(GL_TEXTURE_2D, shadowMapFrameBuffer.depthBuffer);
	//16 bit depth values, 2k resolution 
	glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, 2048, 2048);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//Pixels outside of frustum should have max distance (white)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMapFrameBuffer.depthBuffer, 0);

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
	
	//shadow stuff
	glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFrameBuffer.fbo);
	glViewport(0, 0, 2048, 2048);
	glClear(GL_DEPTH_BUFFER_BIT);

	//set all unifroms for depthonlyShader
	depthonlyShader.use();
	depthonlyShader.setVec3("_LightDirection", lightDirection);
	depthonlyShader.setMat4("_ViewProjection", camera2.projectionMatrix() * camera2.viewMatrix());
	depthonlyShader.setVec3("vPos", camera2.position);

	monkeyModel.draw();
	
	//Draw to off-screen framebuffer instead of screen
	glBindFramebuffer(GL_FRAMEBUFFER, newFrameBuffer.fbo);
	glViewport(0, 0, newFrameBuffer.width, newFrameBuffer.height);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glBindTextureUnit(0, tileTexture);

	//update litShader when adjusting controls
	litShader.use();
	litShader.setMat4("_Model", glm::mat4(1.0f));
	litShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
	litShader.setVec3("_EyePos", camera.position);
	litShader.setFloat("_Material.Ka", material.Ka);
	litShader.setFloat("_Material.Kd", material.Kd);
	litShader.setFloat("_Material.Ks", material.Ks);
	litShader.setFloat("_Material.Shininess", material.Shininess);

	planeMesh.draw();

	//Rotate model around Y axis
	monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

	//transform.modelMatrix() combines translation, rotation, and scale into a 4x4 model matrix
	litShader.setMat4("_Model", monkeyTransform.modelMatrix());

	monkeyModel.draw(); //Draws monkey model using current shader

	 // Draw to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the current post-processing shader
	UpdateCurrentPostProcessingShader();

    // Bind the color buffer texture
    glBindTextureUnit(0, newFrameBuffer.colorBuffer);

    // Draw the quad
    glBindVertexArray(dummyVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6); // 6 for quad, 3 for triangle
	
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
	depthonlyShader = ew::Shader("assets/depthonly.vert", "assets/depthonly.frag");

	//load model
	monkeyModel = ew::Model("assets/suzanne.obj");

	//Bind texture to default in shader 0
	glBindTextureUnit(0, tileTexture);
	//Make "_MainTex" sampler2D sample from the 2D texture bound to unit 0
	litShader.use();
	litShader.setInt("_MainTex", 0);
	litShader.setInt("_MainText_Normal", 1);
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
		resetCamera(currentCamera, &cameraController);
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

	if (ImGui::CollapsingHeader("Active Camera")) {
		//ImGui::Checkbox("Camera 2", &isCamera2);

		if (ImGui::DragFloat3("Direction: ", &lightDirection.x, 0.05, -1, 1)) {
			lightDirection = glm::normalize(lightDirection);
			litShader.setVec3("_LightDirection", lightDirection);
		}
	}


	ImGui::End();

	ImGui::SetNextWindowSize(ImVec2(300, 300));
	ImGui::Begin("Shadow Map");
	//Using a Child allow to fill all the space of the window.
	ImGui::BeginChild("Shadow Map");
	//Stretch image to be window size
	ImVec2 windowSize = ImGui::GetWindowSize();
	//Invert 0-1 V to flip vertically for ImGui display
	//shadowMap is the texture2D handle
	ImGui::Image((ImTextureID)shadowMapFrameBuffer.depthBuffer, windowSize, ImVec2(0, 1), ImVec2(1, 0));
	ImGui::EndChild();
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void CameraSetUp() {
	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	camera2.position = glm::vec3(2.0f, 2.0f, 2.0f);
	camera2.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
	camera2.aspectRatio = (float)screenWidth / screenHeight;
	camera2.fov = 60.0f; //Vertical field of view, in degrees
	camera2.orthographic = true;
	camera2.orthoHeight = 5;
	camera2.aspectRatio = 1;
}

void CameraUpdate(GLFWwindow* window) {
	if (isCamera2) {
		currentCamera = &camera2;
	}
	else {
		currentCamera = &camera;
		//currentCamera = &camera2;
	}

	cameraController.move(window, currentCamera, deltaTime);
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