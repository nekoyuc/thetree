#include <stdio.h>
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "LineRenderer.h"

using namespace glm;

#include "helpers/shader.hpp"
#include "helpers/controls.hpp"

static const GLfloat g_vertex_buffer_data[] = { 
  -1.0f, -1.0f, 0.0f,
  1.0f, -1.0f, 0.0f,
  0.0f,  1.0f, 0.0f,
};

LineRenderer::LineRenderer(int numLines) {
    mNumLines = numLines;
    mVertexBufferData = (GLfloat*)malloc(6 * mNumLines * sizeof(GLfloat));
  glGenVertexArrays(1, &mVertexArrayId);
  glBindVertexArray(mVertexArrayId);
  // Create and compile our GLSL program from the shaders
  mProgramId = LoadShaders( "SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader" );
  glGenBuffers(1, &mVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, mNumVertices * sizeof(GLfloat), mVertexBufferData, GL_STREAM_DRAW);
  mViewProjMatrixId = glGetUniformLocation(mProgramId, "VP");
  mColorId = glGetUniformLocation(mProgramId, "inputColor");
}

LineRenderer::~LineRenderer() {
    free(mVertexBufferData);
  glDeleteBuffers(1, &mVertexBuffer);
  glDeleteVertexArrays(1, &mVertexArrayId);
  glDeleteProgram(mProgramId);
}

glm::mat4 LineRenderer::getMVPMatrix() {
  return getProjectionMatrix()*getViewMatrix();
}

void LineRenderer::addLine(float x1, float y1, float z1, float x2, float y2, float z2) {
    if (mNumVertices + 6 > NUM_LINES * 6) {
        printf("Out of space for lines \n");
        return;
    }

    mVertexBufferData[mNumVertices] = x1;
    mVertexBufferData[mNumVertices + 1] = y1;
    mVertexBufferData[mNumVertices + 2] = z1;
    mVertexBufferData[mNumVertices + 3] = x2;
    mVertexBufferData[mNumVertices + 4] = y2;
    mVertexBufferData[mNumVertices + 5] = z2;
    mNumVertices += 6;

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, mNumVertices * sizeof(GLfloat), mVertexBufferData, GL_STREAM_DRAW);
}

void LineRenderer::uploadToGPU() {
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, mNumVertices * sizeof(GLfloat), mVertexBufferData, GL_STREAM_DRAW);
}

void LineRenderer::render() {
  mModelMatrix = glm::mat4(1.0);
  glm::mat4 ProjectionMatrix = getProjectionMatrix();
  glm::mat4 ViewProjectionMatrix(getMVPMatrix());
  // Use our shader
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(mProgramId);

  glUniform4f(mColorId, mColor[0], mColor[1], mColor[2], mColor[3]);
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
  glLineWidth(2.0);
  glDrawArrays(mMode, 0, mNumVertices/3); // 3 indices starting at 0 -> 1 triangle
  glDisableVertexAttribArray(0);
}


