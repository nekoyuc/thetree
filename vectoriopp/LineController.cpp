#include <stdio.h>
#include "LineController.h"
#include <GLFW/glfw3.h>
#include "helpers/controls.hpp"

#define SCREEN_WIDTH 1024
#define SCREEN_HEIGHT 768

LineController::LineController(LineRenderer* renderer,LineField* field) {
	mLineRenderer = renderer;
	mLineField = field;
}

inline void normalizeCoordinates(double& x, double& y) {
	x /= SCREEN_WIDTH;
	x -= 0.5;
	x *= 2;
	y /= SCREEN_HEIGHT;
	y -= 0.5;
	y *= 2;
}

void LineController::update(GLFWwindow* window, float worldPlaneDistance) {
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
	glm::vec4 coords = { xpos, -ypos, -1.0f, 1.0f };
	glm::vec4 rayEye = glm::inverse(pm) * coords;
	rayEye = { rayEye.x, rayEye.y, rayEye.z, 1.0f };
	glm::vec4 rayWorld4 = (glm::inverse(vm) * rayEye);
	glm::vec3 rayWorld{ rayWorld4.x, rayWorld4.y, rayWorld4.z };
	glm::vec3 direction = rayWorld - getCameraPosition();
	direction = glm::normalize(direction);
	auto end = getCameraPosition() + direction*-worldPlaneDistance;
//	auto end = getCameraPosition() + 3.0f * direction;
	/*auto planeCenterWorld = getCameraPosition() + glm::vec3(0.0f, 0.0f, -3.0f);
	auto dstart = glm::length(planeCenterWorld - getCameraPosition());
	auto dend = glm::length(planeCenterWorld - getCameraPosition() + direction * 100.0f);
	auto distanceToIntersection = dstart / (dstart - dend);
	auto end = getCameraPosition() + direction * distanceToIntersection;
	end[2] = -end[2];
	printf("position: %f %f %f\n", end[0], end[1], end[2]);*/
	
    /*glm::mat4 pm = getProjectionMatrix();
	glm::mat4 vm = getViewMatrix();
	glm::mat4 inverseMatrix = glm::inverse(vm);
	printf("coord %f %f\n", xpos, ypos);
	glm::vec4 coords = { -xpos, ysos, planeDistance, 1.0f };
	glm::vec4 worldPos = inverseMatrix * coords;
	glm::vec3 direction = glm::normalize(glm::vec3(worldPos));
	glm::vec3 start = getCameraPosition();
	glm::vec3 end = start+direction * planeDistance;
	printf("adding line %f %f %f \n", end.x, end.y, end.z);*/
	/*glm::vec4 coords = {xpos, -ypos, 1.0f, 1.0f};
	coords = getProjectionMatrix() * getViewMatrix() * coords;
	auto cameraPosition = glm::vec4(getCameraPosition(), 1.0f);
	auto direction = coords - cameraPosition;
	direction *= 100;
	auto start = cameraPosition;
	auto end = start + direction;
	glm::vec4 planeCenter = { 0.0f,0.0f,planeDistance,1.0f };
	auto dstart = glm::length(planeCenter - start);
	auto dend = glm::length(planeCenter - end);
	auto intersectionDistance = dstart / (dstart - dend);
	auto intersection = start + glm::normalize(direction) * intersectionDistance;
	end = glm::inverse(getProjectionMatrix()*getViewMatrix())*intersection;
	printf("Adding line %f %f %f", end[0], end[1], end[2]);*/

	if (mHasLast) {
		mLineRenderer->addLine(mLastPos[0], mLastPos[1], mLastPos[2], end[0], end[1], end[2]);
		mLineField->recordLine(mLastPos[0], mLastPos[1], mLastPos[2], end[0], end[1], end[2]);
	}
	mHasLast = true;
	mLastPos = end;
	//printf("Adding line (%f, %f, %f) to (%f ,%f, %f)\n", ((xpos / SCREEN_WIDTH) - 1), ((ypos / SCREEN_HEIGHT) - 1), -1.0f,
//		((mLastX / SCREEN_WIDTH) - 1) * 2, ((mLastY / SCREEN_HEIGHT) - 1) * 2, -1.0f);
	// Reset mouse position for next frame
}