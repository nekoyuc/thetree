// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"


glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

// Initial position : on +Z
glm::vec3 position = glm::vec3( 0, 0, 5 );
glm::vec3 getCameraPosition() {
  return position;
}
glm::vec3 direction = glm::vec3( 0, 0, 1);
glm::vec3 getCameraDirection() {
  return direction;
}
glm::vec3 moveDirection = glm::vec3(0, 0, 0);

// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseRotateSpeed = 0.005f;
float mouseMoveSpeed = 0.005f;


float lastX;
float lastY;

void onScrollEvent(GLFWwindow* window, double xoffset, double yoffset) {
	position += direction*(float)yoffset*0.1f;
}

void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	float x = 3.14;
	float* y = &x;

	// Get mouse position
	double xpos, ypos;
	glfwGetCursorPos(window, &xpos, &ypos);

	// Reset mouse position for next frame
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_2)) {
	  mouseRotateSpeed = 0.005;
	  //	  glfwSetCursorPos(window, 1024/2, 768/2);
	} else {
	  mouseRotateSpeed = 0;
	}

	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_3)) {
	  mouseMoveSpeed = 0.1;
	}
	else {
		mouseMoveSpeed = 0;
	}

	// Compute new orientation
	horizontalAngle += -mouseRotateSpeed * float(lastX - xpos );
	verticalAngle   += -mouseRotateSpeed * float( lastY - ypos );

	moveDirection = glm::vec3(lastX - xpos, ypos - lastY, 0);

	lastX = xpos;
	lastY = ypos;

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	direction = glm::vec3(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	
	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	//if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_4)){
	//	position += direction * deltaTime * speed;
	//}

	// Move backward
	//if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_5)) {
	//	position -= direction * deltaTime * speed;
	//}

	position += moveDirection * deltaTime * mouseMoveSpeed;
	//direction = -position; // normalize?

	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_LEFT ) == GLFW_PRESS){
		position -= right * deltaTime * speed;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45?Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								position+direction, // and looks here : at the same position, plus "direction"
								up                  // Head is up (set to 0,-1,0 to look upside-down)
						   );
	//ViewMatrix = glm::lookAt(glm::vec3(0, 0, 0), glm::vec3(0, 0, 1), glm::vec3(0, 1, 0));
	//ViewMatrix = glm::translate(ViewMatrix, position);

	//ViewMatrix = glm::rotate(ViewMatrix, horizontalAngle, glm::vec3(0, 1, 0));
	//ViewMatrix = glm::rotate(ViewMatrix, -verticalAngle/5, glm::vec3(1, 0, 0));

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}
