#include <stdio.h>
#include "LineController.h"
#include <GLFW/glfw3.h>
#include "helpers/controls.hpp"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

LineController::LineController(LineRenderer* renderer,SketchField* field) {
	mLineRenderer = renderer;
	mSketchField = field;
}

inline void normalizeCoordinates(double& x, double& y) {
	x /= SCREEN_WIDTH;
	x -= 0.5;
	x *= 2;
	y /= SCREEN_HEIGHT;
	y -= 0.5;
	y *= 2;
}

void LineController::update(GLFWwindow* window) {
	if (!glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_1)) {
		mHasLast = false;
		return;
	}

	double xpos;
	double ypos;
	glfwGetCursorPos(window, &xpos, &ypos);
	normalizeCoordinates(xpos, ypos);


	glm::mat4 pm = getProjectionMatrix();
	glm::mat4 vm = getViewMatrix();
	glm::mat4 inverseMatrix = glm::inverse(pm * vm);
	glm::vec4 coords = { xpos, -ypos, 1.0f, 1.0f };
	glm::vec4 worldPos = inverseMatrix * coords;
	glm::vec3 direction = glm::normalize(glm::vec3(worldPos));
	glm::vec3 start = getCameraPosition();
	glm::vec3 end = start + direction * 3.0f;

	if (mHasLast) {
		mLineRenderer->addLine(mLastPos[0], mLastPos[1], mLastPos[2], end[0], end[1], end[2]);
		mSketchField->recordLine(mLastPos[0], mLastPos[1], mLastPos[2], end[0], end[1], end[2]);
	}
	mHasLast = true;
	mLastPos = end;
	//printf("Adding line (%f, %f, %f) to (%f ,%f, %f)\n", ((xpos / SCREEN_WIDTH) - 1), ((ypos / SCREEN_HEIGHT) - 1), -1.0f,
//		((mLastX / SCREEN_WIDTH) - 1) * 2, ((mLastY / SCREEN_HEIGHT) - 1) * 2, -1.0f);
	// Reset mouse position for next frame
}