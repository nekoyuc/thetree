#pragma once

#include "LineRenderer.h"
#include "Field.h"
#include <GLFW/glfw3.h>
#include "ParticleSystem.h"

class MouseHandler {
public:
	MouseHandler(LineRenderer*, LineField*, ParticleSystem*);
	~MouseHandler() = default;

	void update(GLFWwindow *window, float planeDistance);
private:
	LineRenderer* mLineRenderer;
	LineField* mLineField;
	ParticleSystem* mParticleSystem;
	glm::vec3 mLastPos;
	bool mHasLast = false;
};