#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;


#include "helpers/shader.hpp"
#include "helpers/texture.hpp"
#include "helpers/controls.hpp"

#include "ParticleSystem.h"
#include "ColoredTriangles.h"
#include "LineRenderer.h"


int main( void )
{
  // Initialise GLFW
  if( !glfwInit() ) {
    fprintf( stderr, "Failed to initialize GLFW\n" );
    getchar();
    return -1;
  }
  glfwWindowHint(GLFW_SAMPLES, 4);
  glfwWindowHint(GLFW_RESIZABLE,GL_FALSE);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  
  // Open a window and create its OpenGL context
  window = glfwCreateWindow( 1024, 768, "Vectorio++", NULL, NULL);
  if( window == NULL ) {
    fprintf( stderr, "Failed to open GLFW window.");
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);

  // Initialize GLEW
  glewExperimental = true; // Needed for core profile
  if (glewInit() != GLEW_OK) {
    fprintf(stderr, "Failed to initialize GLEW\n");
    glfwTerminate();
    return -1;
  }

  // Ensure we can capture the escape key being pressed below
  glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
  // Hide the mouse and enable unlimited mouvement
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  
  // Set the mouse at the center of the screen
  glfwPollEvents();
  glfwSetCursorPos(window, 1024/2, 768/2);
  
  // Dark blue background
  glClearColor(0.0f, 0.0f, 0.4f, 0.0f);
  
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  ParticleSystem particleSystem;
  particleSystem.init();
  GLfloat g_single_triangle_data[] = { 
    -1.0f, -1.0f, 0.0f,
    1.0f, -1.0f, 0.0f,
    0.0f,  1.0f, 0.0f,
  };
  GLfloat g_sample_line_data[] = {
    0.0f, 0.0f, 0.0f,
    0.0f,1.0f, 0.0f,
    0.0f,0.0f,0.0f,
    0.0f,0.0f,1.0f,
    0.0f,0.0f,0.0f,
    1.0f,0.0f,0.0f,
  };
  CameraFacingTriangles drawPlane(&g_single_triangle_data[0], 9);
  LineRenderer drawLines(&g_sample_line_data[0], 18);
  double lastTime = glfwGetTime();
  do {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime;
    lastTime = currentTime;

    particleSystem.update(delta);
    particleSystem.render();
    drawLines.render();
    glDisable(GL_DEPTH_TEST);
    drawPlane.render();
    glEnable(GL_DEPTH_TEST);

    
    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
  } while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

  // Close OpenGL window and terminate GLFW
  glfwTerminate();
  return 0;
}

