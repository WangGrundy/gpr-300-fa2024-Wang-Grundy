#pragma once

#include "../../core/ew/external/glad.h"

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include "../ew/model.h"

#include "../ew/transform.h"
#include <Vector>
#include <iostream>

class Node {

public:
	std::string name;
	ew::Transform globalTransform;
	std::vector<Node*> parents;
	std::vector<Node*> children;
	ew::Model model;

	Node(ew::Model* modelInput) {
		model = *modelInput;
	}

	void AddParent(Node* parentInput) {
		parents.push_back(parentInput);
	}

	void AddChild(Node* childInput) {
		children.push_back(childInput);
	}

	void SetGlobalTransform(ew::Transform transform) {
		globalTransform = transform;
	}

	//void UpdatePosition(ew::Transform transformChange) {

	//	//update this
	//	glm::vec3 posChange = glm::vec3(globalTransform.position.x + transformChange.position.x,
	//		globalTransform.position.y + transformChange.position.y,
	//		globalTransform.position.z + transformChange.position.z);

	//	//update all children nodes
	//	for(Node* node : children) {
	//		UpdatePosition(transformChange);
	//	}
	//}

	void UpdatePosition(glm::vec3 positionChange) {
		//std::cout << name << std::endl;

		//update this
		glm::vec3 posChange = glm::vec3(globalTransform.position.x + positionChange.x,
			globalTransform.position.y + positionChange.y,
			globalTransform.position.z + positionChange.z);

		globalTransform.position = posChange;

		if (children.empty()) {
			return;
		}

		//update all children nodes
		for (Node* node : children) {
			node->UpdatePosition(positionChange);
		}
	}

	void UpdateRotation(glm::quat rotationChangeInput) {
		//std::cout << name << std::endl;

		//update this
		glm::quat rotChange = glm::vec3(globalTransform.rotation.x + rotationChangeInput.x,
			globalTransform.rotation.y + rotationChangeInput.y,
			globalTransform.rotation.z + rotationChangeInput.z);

		globalTransform.rotation = rotChange;

		if (children.empty()) {
			return;
		}

		//update all children nodes
		for (Node* node : children) {
			node->UpdateRotation(rotationChangeInput);
		}
	}
};