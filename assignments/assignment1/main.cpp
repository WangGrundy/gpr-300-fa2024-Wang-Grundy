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

struct Material{
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

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Shader shader;
ew::Shader simpleShader;
ew::Model monkeyModel;

//creating new frame buffer
//wang::Framebuffer myFrameBuffer;

unsigned int framebuffer;
unsigned int textureColorbuffer;

unsigned int quadVAO, quadVBO;
GLuint tileTexture;

int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	LoadModelsAndTextures();
	CameraSetUp();

	float quadVertices[] = { // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
		// positions   // texCoords
		-1.0f,  1.0f,  0.0f, 1.0f,
		-1.0f, -1.0f,  0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,

		-1.0f,  1.0f,  0.0f, 1.0f,
		 1.0f, -1.0f,  1.0f, 0.0f,
		 1.0f,  1.0f,  1.0f, 1.0f
	};
	// screen quad VAO
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	//--------------------------------------------------------------

	//create an actual framebuffer object and bind it
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	//create a texture image that we attach as a color attachment to the framebuffer
	//set the texture's dimensions equal to the width and height of the window 
	// generate texture
	glGenTextures(1, &textureColorbuffer);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 800, 600, 0, GL_RGB, GL_UNSIGNED_INT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//glBindTexture(GL_TEXTURE_2D, 0);

	// attach it to currently bound framebuffer object
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureColorbuffer, 0);

	//creating a renderBuffer object 
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, screenWidth, screenHeight);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	//attach the renderbuffer object to the depth and stencil attachment of the framebuffer
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

	//frame buffer checker
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	/*So, to draw the scene to a single texture we'll have to take the following steps:

		Render the scene as usual with the new framebuffer bound as the active framebuffer.
		Bind to the default framebuffer.
		Draw a quad that spans the entire screen with the new framebuffer's color buffer as its texture.*/

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

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	//configure VAO screen quad:
	// first pass
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glClearColor(0.6f, 0.8f, 0.9f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // we're not using the stencil buffer now
	glEnable(GL_DEPTH_TEST);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tileTexture);
	
	//Bind brick texture to texture unit 0 
	glBindTextureUnit(0, tileTexture);
	//Make "_MainTex" sampler2D sample from the 2D texture bound to unit 0
	shader.use();
	shader.setInt("_MainTex", 0);
	shader.setInt("_MainText_Normal", 1);

	glBindVertexArray(quadVAO);
	glDisable(GL_DEPTH_TEST);
	glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//Rotate model around Y axis
	monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

	//transform.modelMatrix() combines translation, rotation, and scale into a 4x4 model matrix
	shader.setMat4("_Model", monkeyTransform.modelMatrix());

	simpleShader.use();

	//// second pass
	//glBindFramebuffer(GL_FRAMEBUFFER, 0); // back to default
	//glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	//glClear(GL_COLOR_BUFFER_BIT);

	monkeyModel.draw(); //Draws monkey model using current shader

	//glDisable(GL_DEPTH_TEST);
	//glBindVertexArray(quadVAO);
	//glBindTexture(GL_TEXTURE_2D, textureColorbuffer);

	//glDrawArrays(GL_TRIANGLES, 0, 6);
}

void LoadModelsAndTextures() {

	//Handles to OpenGL object are unsigned integers
	tileTexture = ew::loadTexture("assets/materials/Tile/Tiles130.jpg");

	//load shader //load model
	shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	simpleShader = ew::Shader("assets/simple.vert", "assets/simple.frag");
	monkeyModel = ew::Model("assets/suzanne.obj");
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

	ImGui::End();

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
