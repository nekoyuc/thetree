#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>

#include "ColoredTriangles.h"

using namespace glm;

#include "helpers/shader.hpp"
#include "helpers/controls.hpp"

static const GLfloat g_vertex_buffer_data[] = { 
  -1.0f, -1.0f, 0.0f,
  1.0f, -1.0f, 0.0f,
  0.0f,  1.0f, 0.0f,
};

ColoredTriangles::ColoredTriangles(GLfloat* vertexBufferData, int numVertices) :
  mVertexBufferData(vertexBufferData), mNumVertices(numVertices) {
  glGenVertexArrays(1, &mVertexArrayId);
  glBindVertexArray(mVertexArrayId);
  // Create and compile our GLSL program from the shaders
  mProgramId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );
  glGenBuffers(1, &mVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, mNumVertices*sizeof(GLfloat), mVertexBufferData, GL_STATIC_DRAW);
  mViewProjMatrixId = glGetUniformLocation(mProgramId, "VP");
}

ColoredTriangles::~ColoredTriangles() {
  glDeleteBuffers(1, &mVertexBuffer);
  glDeleteVertexArrays(1, &mVertexArrayId);
  glDeleteProgram(mProgramId);
}

void ColoredTriangles::render() {
  glm::mat4 ProjectionMatrix = getProjectionMatrix();
  glm::mat4 ViewMatrix = getViewMatrix();
  glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;
  // Use our shader
  glUseProgram(mProgramId);

  glUniformMatrix4fv(mViewProjMatrixId, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);
  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  glVertexAttribPointer(
    0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
    3,                  // size
    GL_FLOAT,           // type
    GL_FALSE,           // normalized?
    0,                  // stride
    (void*)0            // array buffer offset
  );
  glDrawArrays(GL_TRIANGLES, 0, 3); // 3 indices starting at 0 -> 1 triangle
  glDisableVertexAttribArray(0);
}
  

