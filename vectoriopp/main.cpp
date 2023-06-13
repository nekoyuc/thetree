#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>
#include <memory>

#include <GL/glew.h>

#include <GLFW/glfw3.h>
#include <chrono>
GLFWwindow* window;

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;


#include "helpers/shader.hpp"
#include "helpers/texture.hpp"
#include "helpers/controls.hpp"

#include "ColoredTriangles.h"
#include "DensityGrid.h"
#include "DensityViz.h"
#include "LineRenderer.h"
#include "MouseHandler.h"
#include "ParticleSystem.h"
#include "Vectorio.h"


GLfloat g_single_triangle_data[] = {
  -1.0f, -1.0f, 0.0f,
  1.0f, -1.0f, 0.0f,
  0.0f,  1.0f, 0.0f,
};

/*
GLfloat g_sample_line_data[] = {
  0.0f, 0.0f, 0.0f,
  0.0f,1.0f, 0.0f,
  0.0f,0.0f,0.0f,
  0.0f,0.0f,1.0f,
  0.0f,0.0f,0.0f,
  1.0f,0.0f,0.0f,
};
*/



int main()
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
  window = glfwCreateWindow(1024, 768, "Vectorio++", NULL, NULL);
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
  // Hide the mouse and enable unlimited movement
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
  
  // Set the mouse at the center of the screen
  glfwPollEvents();
  glfwSetCursorPos(window, 1024/2, 768/2);
  
  Vectorio vectorio;
  MouseHandler mouseHandler(&vectorio);

  glfwSetScrollCallback(window, onScrollEvent);


  double lastTime = glfwGetTime();
  std::future<std::vector<DensityGrid::Entry>> futureProfiling;
  bool waitingOnFuture = false;
  bool drawPlaneRender = true;

  printf("\n**press WS to move draw plane\n");
  printf("**press AD to change field of view\n");
  printf("**press QE to change profiling threshold, default = 400, press left shift to speed up\n");
  printf("**press X to change trail visibility\n");
  printf("**press C to change draw plane visibility\n");
  printf("**press Z to make density profile\n");
  printf("**press SPACE to erase particles\n");
  printf("**press 1 to flip rotation field\n\n");
  //printf("current rotation direction is %f\n\n", lineField.rotation_direction);

  do {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime; 
    lastTime = currentTime;


    mouseHandler.update(window, vectorio.getDrawPlaneDistance());
    vectorio.update(delta, getProjectionMatrix(), getViewMatrix());
    
    vectorio.render(getProjectionMatrix()*getViewMatrix());
       
    if (waitingOnFuture) {
        auto available = futureProfiling.wait_for(std::chrono::nanoseconds(1));
        if (available == std::future_status::ready) {
            vectorio.visualizeField(futureProfiling.get());
            //densityGrid->doneProfiling();
            waitingOnFuture = false;
            printf("done profiling\n\n");
        }
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        vectorio.moveDrawPlane(glm::vec3(0.0f, 0.0f, -0.05f));
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        vectorio.moveDrawPlane(glm::vec3(0.0f, 0.0f, 0.05f));
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        vectorio.toggleShowTrails();
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        vectorio.toggleRenderDrawPlane();
    }

    if ((glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) && !waitingOnFuture) {
        futureProfiling = vectorio.profile();
        waitingOnFuture = true;
    }

    if ((glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS)) {
        vectorio.changeProfileThreshold(0.5f);
    }

    if ((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS)) {
        vectorio.changeProfileThreshold(-0.5f);
    }

    if ((glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {
        vectorio.changeProfileThreshold(-5.0f);
    }

    if ((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {
        vectorio.changeProfileThreshold(5.0f);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        vectorio.setEraseOn(true);
    } else {
        vectorio.setEraseOn(false);
    }
  
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        vectorio.invertLineField();
    }

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
    
    computeMatricesFromInputs();
  } while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );                                                                                              

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  return 0;
}

