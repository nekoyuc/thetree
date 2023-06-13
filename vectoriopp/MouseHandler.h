#pragma once

#include "LineRenderer.h"
#include "Field.h"
#include <GLFW/glfw3.h>

class Vectorio;

class MouseHandler {
public:
	MouseHandler(Vectorio*);
	~MouseHandler() = default;

	void update(GLFWwindow *window, float planeDistance);
private:
	Vectorio* mVectorio;
	glm::vec3 mLastPos;
	bool mHasLast = false;
};