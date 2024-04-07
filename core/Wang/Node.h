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
};