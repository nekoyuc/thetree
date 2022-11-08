#include <GLFW/glfw3.h>

#ifndef CONTROLS_HPP
#define CONTROLS_HPP

// GLFW callback
void onScrollEvent(GLFWwindow* window, double xoffset, double yoffset);

void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
glm::vec3 getCameraPosition();
glm::vec3 getCameraDirection();

#endif
