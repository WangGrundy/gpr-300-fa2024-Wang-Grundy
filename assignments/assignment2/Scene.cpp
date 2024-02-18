#include "Scene.h"

void wang::Scene::RenderInMain() {

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

void wang::Scene::LoadModelsAndTextures() {
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

void wang::Scene::UpdateCurrentPostProcessingShader() {

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

void wang::Scene::CameraSetUp() {
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

void  wang::Scene::CameraUpdate(GLFWwindow* window) {
	if (isCamera2) {
		currentCamera = &camera2;
	}
	else {
		currentCamera = &camera;
		//currentCamera = &camera2;
	}

	cameraController.move(window, currentCamera, deltaTime);
}