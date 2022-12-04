#pragma once

#include "LineRenderer.h"
#include "Field.h"
#include <GLFW/glfw3.h>

class LineController {
public:
	LineController(LineRenderer*,LineField*);
	~LineController() = default;

	void update(GLFWwindow *window, float planeDistance);
private:
	LineRenderer* mLineRenderer;
	LineField* mLineField;
	glm::vec3 mLastPos;
	bool mHasLast = false;
};