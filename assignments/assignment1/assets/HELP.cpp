//#include <stdio.h>
//#include <math.h>
//#include <iostream>
//
//#include <ew/external/glad.h>
//
//#include <GLFW/glfw3.h>
//#include <imgui.h>
//#include <imgui_impl_glfw.h>
//#include <imgui_impl_opengl3.h>
//
//#include <ew/shader.h>
//#include <ew/model.h>
//#include <ew/camera.h>
//#include <ew/transform.h>
//#include <ew/cameraController.h>
//#include <ew/texture.h>
//#include <Wang/framebuffer.h>
//
//#include <Wang/framebuffer.h>
//
//struct Material {
//	float Ka = 1.0;
//	float Kd = 0.5;
//	float Ks = 0.5;
//	float Shininess = 128;
//}material;
//
//ew::CameraController cameraController;
//ew::Transform monkeyTransform;
//ew::Camera camera;
//
//void framebufferSizeCallback(GLFWwindow* window, int width, int height);
//GLFWwindow* initWindow(const char* title, int width, int height);
//void drawUI();
//void RenderInMain();
//void CameraUpdate(GLFWwindow* window);
//void CameraSetUp();
//void LoadModelsAndTextures();
//void UpdateCurrentPostProcessingShader();
//
////Global state
//int screenWidth = 1080;
//int screenHeight = 720;
//float prevFrameTime;
//float deltaTime;
//
//ew::Shader currentPostProcessingShader, litShader, invertShader, blurShader, noShader;
//ew::Model monkeyModel;
//
//unsigned int dummyVAO;
//
//wang::Framebuffer newFrameBuffer;
//GLuint tileTexture;
//
//bool noPostProcessShader = true, isInvertShaderEnabled = false, isBlurShaderEnabled = false;
//bool postProcessShaderChanged = false;
//
//int main() {
//	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
//	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
//
//	glEnable(GL_CULL_FACE);
//	glCullFace(GL_BACK); //Back face culling
//	glEnable(GL_DEPTH_TEST); //Depth testing	
//
//	LoadModelsAndTextures();
//	CameraSetUp();
//
//	//creasting dummy VAO
//	glCreateVertexArrays(1, &dummyVAO);
//
//	newFrameBuffer = wang::createFramebuffer(screenWidth, screenHeight, 1); //no idea what color colorFormat means
//
//	while (!glfwWindowShouldClose(window)) {
//		glfwPollEvents();
//
//		float time = (float)glfwGetTime();
//		deltaTime = time - prevFrameTime;
//		prevFrameTime = time;
//
//		CameraUpdate(window);
//		UpdateCurrentPostProcessingShader();
//
//		RenderInMain();
//		drawUI();
//
//		glfwSwapBuffers(window);
//	}
//	printf("Shutting down...");
//}
//
//void RenderInMain() {
//
//	//Draw to off-screen framebuffer instead of screen
//	glBindFramebuffer(GL_FRAMEBUFFER, newFrameBuffer.fbo);
//	glViewport(0, 0, screenWidth, screenHeight); //IDK IF THE VIEW PORT SHOULD BE THIS SIZE
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//	//this is lit shader
//	litShader.use();
//	litShader.setMat4("_Model", glm::mat4(1.0f));
//	litShader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
//	litShader.setVec3("_EyePos", camera.position);
//
//	litShader.setFloat("_Material.Ka", material.Ka);
//	litShader.setFloat("_Material.Kd", material.Kd);
//	litShader.setFloat("_Material.Ks", material.Ks);
//	litShader.setFloat("_Material.Shininess", material.Shininess);
//
//	//Rotate model around Y axis
//	monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));
//
//	//transform.modelMatrix() combines translation, rotation, and scale into a 4x4 model matrix
//	litShader.setMat4("_Model", monkeyTransform.modelMatrix());
//
//	monkeyModel.draw(); //Draws monkey model using current shader
//
//	//Draw to default framebuffer
//	glBindFramebuffer(GL_FRAMEBUFFER, 0);
//	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//	currentPostProcessingShader.use();
//	glBindTextureUnit(0, newFrameBuffer.fbo);
//	glBindVertexArray(dummyVAO);
//	glDrawArrays(GL_TRIANGLES, 0, 6); //6 for quad, 3 for triangle
//}
//
//void LoadModelsAndTextures() {
//
//	//Handles to OpenGL object are unsigned integers
//	tileTexture = ew::loadTexture("assets/materials/Tile/Tiles130.jpg");
//
//	//load shader //load model
//	litShader = ew::Shader("assets/lit.vert", "assets/lit.frag");
//	invertShader = ew::Shader("assets/invert.vert", "assets/invert.frag");
//	blurShader = ew::Shader("assets/blur.vert", "assets/blur.frag");
//	noShader = ew::Shader("assets/none.vert", "assets/none.frag");
//
//	monkeyModel = ew::Model("assets/suzanne.obj");
//
//	//Bind brick texture to texture unit 0 
//	glBindTextureUnit(0, tileTexture);
//
//	//Make "_MainTex" sampler2D sample from the 2D texture bound to unit 0
//	litShader.use();
//	litShader.setInt("_MainTex", 0);
//	litShader.setInt("_MainText_Normal", 1);
//
//	//set initial shader
//	currentPostProcessingShader = noShader;
//}
//
//void UpdateCurrentPostProcessingShader() {
//
//	if (!postProcessShaderChanged) {
//		return;
//	}
//	postProcessShaderChanged = false;
//
//	if (noPostProcessShader) {
//		currentPostProcessingShader = noShader;
//	}
//	else if (isInvertShaderEnabled) {
//		currentPostProcessingShader = invertShader;
//	}
//	else if (isBlurShaderEnabled) {
//		currentPostProcessingShader = blurShader;
//	}
//	else {
//		//default
//		currentPostProcessingShader = noShader;
//	}
//
//}
//
//void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
//	camera->position = glm::vec3(0, 0, 5.0f);
//	camera->target = glm::vec3(0);
//	controller->yaw = controller->pitch = 0;
//}
//
//void drawUI() {
//	ImGui_ImplGlfw_NewFrame();
//	ImGui_ImplOpenGL3_NewFrame();
//	ImGui::NewFrame();
//
//	ImGui::Begin("Settings");
//	ImGui::Text("Add Controls Here!");
//
//	if (ImGui::Button("Reset Camera")) {
//		resetCamera(&camera, &cameraController);
//	}
//
//	if (ImGui::CollapsingHeader("Material")) {
//		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
//		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
//		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
//		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
//	}
//
//	if (ImGui::CollapsingHeader("Post Processing Effects")) {
//		ImGui::Checkbox("none", &noPostProcessShader);
//		ImGui::Checkbox("Invert", &isInvertShaderEnabled);
//		ImGui::Checkbox("Blur", &isBlurShaderEnabled);
//
//		postProcessShaderChanged = true;
//	}
//	ImGui::End();
//
//	ImGui::Render();
//	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
//}
//
//void CameraSetUp() {
//	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
//	camera.target = glm::vec3(0.0f, 0.0f, 0.0f); //Look at the center of the scene
//	camera.aspectRatio = (float)screenWidth / screenHeight;
//	camera.fov = 60.0f; //Vertical field of view, in degrees
//}
//
//void CameraUpdate(GLFWwindow* window) {
//	cameraController.move(window, &camera, deltaTime);
//}
//
//void framebufferSizeCallback(GLFWwindow* window, int width, int height)
//{
//	glViewport(0, 0, width, height);
//	screenWidth = width;
//	screenHeight = height;
//}
//
///// <summary>
///// Initializes GLFW, GLAD, and IMGUI
///// </summary>
///// <param name="title">Window title</param>
///// <param name="width">Window width</param>
///// <param name="height">Window height</param>
///// <returns>Returns window handle on success or null on fail</returns>
//GLFWwindow* initWindow(const char* title, int width, int height) {
//	printf("Initializing...");
//	if (!glfwInit()) {
//		printf("GLFW failed to init!");
//		return nullptr;
//	}
//
//	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
//	if (window == NULL) {
//		printf("GLFW failed to create window");
//		return nullptr;
//	}
//	glfwMakeContextCurrent(window);
//
//	if (!gladLoadGL(glfwGetProcAddress)) {
//		printf("GLAD Failed to load GL headers");
//		return nullptr;
//	}
//
//	//Initialize ImGUI
//	IMGUI_CHECKVERSION();
//	ImGui::CreateContext();
//	ImGui_ImplGlfw_InitForOpenGL(window, true);
//	ImGui_ImplOpenGL3_Init();
//
//	return window;
//}