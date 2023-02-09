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

GLfloat plane_data[] = {
-3.0f, -3.0f, 0.0f,
3.0f, -3.0f, 0.0f,
3.0f,  3.0f, 0.0f,
-3.0f, -3.0f, 0.0f,
3.0f, 3.0f, 0.0f,
-3.0f, 3.0f, 0.0f
};

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
  
  // Light blue background
  glClearColor(0.7f, 0.8f, 0.9f, 0.0f);
  
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);

  DensityGrid* densityGrid = new DensityGrid();
  ParticleSystem* particleSystem = new ParticleSystem(densityGrid);

  CameraFacingTriangles drawPlane(&plane_data[0], 18);
  drawPlane.mPosition = glm::vec3(0.0f, 0.0f, -3.0f);
  drawPlane.mColor = glm::vec4(0.9, 0.9, 0.9, 0.4);

  struct Plane : public ColoredTriangles {
      glm::mat4 mModelMatrix = glm::mat4(1.0f);
      Plane() : ColoredTriangles(&plane_data[0], 18) {
          mModelMatrix = glm::rotate(mModelMatrix, (float)(M_PI / 2.0f), glm::vec3(1, 0, 0));
          mColor = glm::vec4(0.2, 0.2, 0.2, 0.97);
      }
      glm::mat4 getMVPMatrix() override {
          return getProjectionMatrix() * getViewMatrix() * mModelMatrix;
      }
  };
  Plane basePlane;

  LineRenderer lineRenderer;
  lineRenderer.addLine(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
  lineRenderer.addLine(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
  lineRenderer.addLine(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);

  LineField lineField;  
  MouseHandler mouseHandler(&lineRenderer, &lineField, particleSystem);

  glfwSetScrollCallback(window, onScrollEvent);

  auto densityVisualizer = std::make_unique<DensityViz>();
  bool hasDensityVisualization = false;


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
  printf("current rotation direction is %f\n\n", lineField.rotation_direction);

  do {
    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    double currentTime = glfwGetTime();
    double delta = currentTime - lastTime; 
    lastTime = currentTime;

    basePlane.render();
    glDisable(GL_DEPTH_TEST);

    if (hasDensityVisualization) {
        densityVisualizer->render();
    }

    mouseHandler.update(window, drawPlane.mPosition.z);

    particleSystem->update(delta, &lineField);
    glDisable(GL_BLEND);
    if (!(glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)) {
        particleSystem->render();
    }
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    lineRenderer.render();
    if (drawPlaneRender == true) {
        drawPlane.render();
    }

   
    if (waitingOnFuture) {
        auto available = futureProfiling.wait_for(std::chrono::nanoseconds(1));
        if (available == std::future_status::ready) {
            densityVisualizer->visualizeField(futureProfiling.get());
            hasDensityVisualization = true;
            //densityGrid->doneProfiling();
            waitingOnFuture = false;
            printf("done profiling\n\n");
        }
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        drawPlane.mPosition -= glm::vec3(0.0f, 0.0f, 0.05f);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        drawPlane.mPosition += glm::vec3(0.0f, 0.0f, 0.05f);
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        particleSystem->showTrail = !particleSystem->showTrail;
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        drawPlaneRender = !drawPlaneRender;
    }

    if ((glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) && !waitingOnFuture) {
        futureProfiling = densityGrid->profile();
        waitingOnFuture = true;
    }

    if ((glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS)) {
        densityGrid->threshold -= 0.5f;
        printf("threshold is %f\n", densityGrid->threshold);
    }

    if ((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) != GLFW_PRESS)) {
        densityGrid->threshold += 0.5f;
        printf("threshold is %f\n", densityGrid->threshold);
    }

    if ((glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {
        densityGrid->threshold -= 5.0f;
        printf("threshold is %f\n", densityGrid->threshold);
    }

    if ((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) && (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)) {
        densityGrid->threshold += 5.0f;
        printf("threshold is %f\n", densityGrid->threshold);
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        particleSystem->eraseOn = true;
    }
    else {
        particleSystem->eraseOn = false;
    }
  
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        lineField.rotation_direction *= -1.0f;
        printf("\n\ncurrent rotation direction is %f", lineField.rotation_direction);
    }

    // Swap buffers
    glfwSwapBuffers(window);
    glfwPollEvents();
    
    computeMatricesFromInputs();
  } while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );                                                                                              

  // Close OpenGL window and terminate GLFW
  glfwTerminate();

  delete densityGrid;
  return 0;
}

