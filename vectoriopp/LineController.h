#pragma once

#include "LineRenderer.h"
#include "SketchField.h"
#include <GLFW/glfw3.h>

class LineController {
public:
	LineController(LineRenderer*,SketchField*);
	~LineController() = default;

	void update(GLFWwindow *window);
private:
	LineRenderer* mLineRenderer;
	SketchField* mSketchField;
	glm::vec3 mLastPos;
	bool mHasLast = false;
};