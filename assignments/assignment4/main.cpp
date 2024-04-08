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
#include <Wang/Node.h>
#include <vector>
#include <map>

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

void CreateNodes();
void CreateAnimationModels();
void DrawAllInTree(Node* parent);
void AnimationMovement();
float BounceUntil(bool& isIncreasing, float& current, float min, float max, float amount);

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

int HDR = 0;

ew::Shader litShader, invertShader, blurShader, noShader, gammaShader, grayscaleShader, coldShader;
bool shaderChanged = false;
ew::Model monkeyModel;

std::vector<ew::Model> animationModels;

unsigned int dummyVAO;

wang::Framebuffer newFrameBuffer;
GLuint tileTexture;

float gammaVal = 0;

bool isColdShaderEnabled = false, isGrayscaleShaderEnabled = false, noPostProcessShader = false, isInvertShaderEnabled = false, isBlurShaderEnabled = false, isGammaShaderEnabled = false;
bool postProcessShaderChanged = false;

Node* rootOfAnimation = nullptr;

ew::Model* headModel;
ew::Model* bodyModel;
ew::Model* waistModel;

ew::Model* leftKneeModel;
ew::Model* leftFootModel;

ew::Model* rightKneeModel;
ew::Model* rightFootModel;

ew::Model* leftElbowModel;
ew::Model* leftHandModel;
ew::Model* rightElbowModel;
ew::Model* rightHandModel;

/////

Node* head;
Node* body;
Node* waist;

Node* leftKnee;
Node* leftFoot;
Node* rightKnee;
Node* rightFoot;

Node* leftElbow;
Node* leftHand;
Node* rightElbow;
Node* rightHand;

//

glm::quat rightElbowRotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
float rightElbowRotationAmount = 0;

bool rightElbowPosYIncreasing = false;
float rightElbowPosYChange = 0.f;
glm::vec3 rightElbowOriginalPos;

bool rightElbowPosZIncreasing = false;
float rightElbowPosZChange = 0.f;

bool leftElbowPosDecreasing = false;

float headScale = 2;
bool isHeadDecreasing = false;



int main() {
	GLFWwindow* window = initWindow("Assignment 1", screenWidth, screenHeight);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK); //Back face culling
	glEnable(GL_DEPTH_TEST); //Depth testing

	LoadModelsAndTextures();
	CameraSetUp();

	newFrameBuffer = wang::createFramebuffer(screenWidth, screenHeight, GL_RGB32F);
	glCreateVertexArrays(1, &dummyVAO);

	CreateAnimationModels();
	CreateNodes();

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

void CreateNodes() {
	head = new Node(headModel); head->name = "head";
	body = new Node(bodyModel); body->name = "body";
	waist = new Node(waistModel); waist->name = "waist";

	leftKnee = new Node(leftKneeModel); leftKnee->name = "leftKnee";
	leftFoot = new Node(leftFootModel); leftFoot->name = "leftFoot";
	rightKnee = new Node(rightKneeModel); rightKnee->name = "rightKnee";
	rightFoot = new Node(rightFootModel); rightFoot->name = "rightFoot";

	leftElbow = new Node(leftElbowModel); leftElbow->name = "leftElbow";
	leftHand = new Node(leftHandModel); leftHand->name = "leftHand";
	rightElbow = new Node(rightElbowModel); rightElbow->name = "rightElbow";
	rightHand = new Node(rightHandModel); rightHand->name = "rightHand";

	rootOfAnimation = head;

	//upper
	head->AddChild(body);

	body->AddParent(head);
	body->AddChild(waist);
	body->AddChild(leftElbow);
	body->AddChild(rightElbow);

	waist->AddParent(body);
	waist->AddChild(leftKnee);
	waist->AddChild(rightKnee);

	//arms and hands
	leftElbow->AddParent(body);
	leftElbow->AddChild(leftHand);
	
	leftHand->AddParent(leftElbow);

	rightElbow->AddParent(body);
	rightElbow->AddChild(rightHand);
	
	rightHand->AddParent(rightElbow);

	//legs and knees
	leftKnee->AddParent(waist);
	leftKnee->AddChild(leftFoot);
	
	leftFoot->AddParent(leftKnee);

	rightKnee->AddParent(waist);
	rightKnee->AddChild(rightFoot);

	rightFoot->AddParent(rightKnee);

	////////////////////////////////////////////////////////

	glm::vec3 pos = glm::vec3(0, 0, 0);
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	ew::Transform transform;

	//main 3
	{
		transform.position = pos;
		transform.rotation = rotation;
		transform.scale = glm::vec3(2.0f, 2.0f, 2.0f);;

		head->SetGlobalTransform(transform);

		//
		glm::vec3 headPos = head->globalTransform.position;
		transform.position = glm::vec3(headPos.x, headPos.y - 5, headPos.z);
		transform.rotation = rotation;
		transform.scale = scale;

		body->SetGlobalTransform(transform);

		//
		glm::vec3 bodyPos = body->globalTransform.position;
		transform.position = glm::vec3(bodyPos.x, bodyPos.y - 5, bodyPos.z);
		transform.rotation = rotation;
		transform.scale = scale;

		waist->SetGlobalTransform(transform);
	}
	
	//legs
	{
		glm::vec3 waistPos = waist->globalTransform.position;
		transform.position = glm::vec3(waistPos.x - 3, waistPos.y - 6, waistPos.z);
		transform.rotation = rotation;
		transform.scale = scale;

		leftKnee->SetGlobalTransform(transform);

		//
		glm::vec3 leftKneePos = leftKnee->globalTransform.position;
		transform.position = glm::vec3(leftKneePos.x, leftKneePos.y - 5, leftKneePos.z);
		transform.rotation = rotation;
		transform.scale = scale;
		
		leftFoot->SetGlobalTransform(transform);

		//
		transform.position = glm::vec3(waistPos.x + 3, waistPos.y - 6, waistPos.z);
		transform.rotation = rotation;
		transform.scale = scale;

		rightKnee->SetGlobalTransform(transform);

		//
		glm::vec3 rightKneePos = rightKnee->globalTransform.position;
		transform.position = glm::vec3(rightKneePos.x, rightKneePos.y - 5, rightKneePos.z);
		transform.rotation = rotation;
		transform.scale = scale;

		rightFoot->SetGlobalTransform(transform);
	}

	//arms
	{
		glm::vec3 bodyPos = body->globalTransform.position;
		transform.position = glm::vec3(bodyPos.x - 4, bodyPos.y, bodyPos.z);
		transform.rotation = rotation;
		transform.scale = scale;

		leftElbow->SetGlobalTransform(transform);

		glm::vec3 leftElbowPos = leftElbow->globalTransform.position;
		transform.position = glm::vec3(leftElbowPos.x - 2, leftElbowPos.y - 3, leftElbowPos.z);
		transform.rotation = rotation;
		transform.scale = scale;

		leftHand->SetGlobalTransform(transform);
		//

		transform.position = glm::vec3(bodyPos.x + 4, bodyPos.y, bodyPos.z);
		transform.rotation = rotation;
		transform.scale = scale;

		rightElbow->SetGlobalTransform(transform);

		glm::vec3 rightElbowPos = rightElbow->globalTransform.position;
		transform.position = glm::vec3(rightElbowPos.x + 2, rightElbowPos.y - 3, rightElbowPos.z);
		transform.rotation = rotation;
		transform.scale = scale;

		rightHand->SetGlobalTransform(transform);
	}

	///////////////////////
	rightElbowOriginalPos = rightElbow->globalTransform.position;
}

void CreateAnimationModels() {
	headModel = new ew::Model("assets/suzanne.obj");
	bodyModel = new ew::Model("assets/suzanne.obj");
	waistModel = new ew::Model("assets/suzanne.obj");

	leftKneeModel = new ew::Model("assets/suzanne.obj");
	leftFootModel = new ew::Model("assets/suzanne.obj");

	rightKneeModel = new ew::Model("assets/suzanne.obj");
	rightFootModel = new ew::Model("assets/suzanne.obj");

	leftElbowModel = new ew::Model("assets/suzanne.obj");
	leftHandModel = new ew::Model("assets/suzanne.obj");
	rightElbowModel = new ew::Model("assets/suzanne.obj");
	rightHandModel = new ew::Model("assets/suzanne.obj");
}

void RenderInMain() {
	
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

	//Rotate model around Y axis
	monkeyTransform.rotation = glm::rotate(monkeyTransform.rotation, deltaTime, glm::vec3(0.0, 1.0, 0.0));

	DrawAllInTree(rootOfAnimation);
	AnimationMovement();
	
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

void AnimationMovement() {

	glm::vec3 moveEveryFrame(0.001, 0, 0);
	head->UpdatePosition(moveEveryFrame);

	//arms and elbows
	{
		rightElbowRotationAmount += 0.2 * deltaTime;

		rightElbowRotation = glm::angleAxis(glm::degrees(rightElbowRotationAmount), glm::vec3(1.0f, 1.0f, 0.0f));
		rightElbow->UpdateRotation(rightElbowRotation);

		rightElbowRotation = glm::angleAxis(glm::degrees(rightElbowRotationAmount), glm::vec3(-1.0f, -1.0f, 0.0f));
		leftElbow->UpdateRotation(rightElbowRotation);

		////increase and decrease arm z and y pos (max = 2)s
		//float changeY = BounceUntil(rightElbowPosYIncreasing, rightElbowPosYChange, -2.f, 2.f, 0.002f);
		//std::cout << changeY << std::endl;

		{
			//right elbow increase/decrease
			if (rightElbowPosYIncreasing) {

				rightElbowPosYChange += 4 * deltaTime;
				glm::vec3 newPosY(0, 4 * deltaTime, 0);
				rightElbow->UpdatePosition(newPosY);

				//if it reaches desired max, go the other way
				if (rightElbowPosYChange >= 1) {
					rightElbowPosYIncreasing = false;
				}
			}
			else {
				rightElbowPosYChange -= 4 * deltaTime;
				glm::vec3 newPosY(0, -4 * deltaTime, 0);
				rightElbow->UpdatePosition(newPosY);

				//if it reaches desired max, go the other way
				if (rightElbowPosYChange <= -1) {
					rightElbowPosYIncreasing = true;
				}
			}
		}
		
		{
			if (rightElbowPosZIncreasing) {

				rightElbowPosZChange += 4 * deltaTime;
				glm::vec3 newPosY(0, 0, 4 * deltaTime);
				rightElbow->UpdatePosition(newPosY);

				//if it reaches desired max, go the other way
				if (rightElbowPosZChange >= 1) {
					rightElbowPosZIncreasing = false;
				}
			}
			else {
				rightElbowPosZChange -= 4 * deltaTime;
				glm::vec3 newPosY(0, 0, -4 * deltaTime);
				rightElbow->UpdatePosition(newPosY);

				//if it reaches desired max, go the other way
				if (rightElbowPosZChange <= -1) {
					rightElbowPosZIncreasing = true;
				}
			}
		}
	}

	//big 3
	{
		if (isHeadDecreasing) {
			headScale -= 0.01;
			if (headScale <= 2) {
				isHeadDecreasing = false;
			}
		}
		else {
			headScale += 0.01;
			if (headScale >= 5) {
				isHeadDecreasing = true;
			}
		}

		head->globalTransform.scale = glm::vec3(headScale, headScale, headScale);
	}

}

void DrawAllInTree(Node* current) {
	if (current->children.empty())
	{
		litShader.setMat4("_Model", current->globalTransform.modelMatrix());
		current->model.draw();
		return;
	}

	//keep exploring until no children
	for (Node* node : current->children) 
	{
		DrawAllInTree(node);
	}

	//draw this
	litShader.setMat4("_Model", current->globalTransform.modelMatrix());
	current->model.draw();
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

float BounceUntil(bool& isIncreasing, float& current, float min, float max, float amount) {
	
	if (isIncreasing) {
		current += amount;

		if (current >= max) {
			isIncreasing = false;
		}
	}
	else {
		current -= amount;

		if (current <= min) {
			isIncreasing = true;
		}
	}

	return current;
}