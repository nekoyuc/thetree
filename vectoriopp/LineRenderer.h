#pragma once
#include <GL/glew.h>

#include <glm/glm.hpp>

#define NUM_LINES 10000

class LineRenderer {
public:
  LineRenderer(int numLines = NUM_LINES);
  virtual ~LineRenderer();
  LineRenderer(const LineRenderer& other) = delete;

  void addLine(float x1, float y1, float z1, float x2, float y2, float z2);
  void uploadToGPU();

  void render();
  virtual glm::mat4 getMVPMatrix();
  GLfloat* mVertexBufferData;
  GLuint mMode = GL_LINES;
  int mNumVertices = 0;

  glm::vec4 mColor = glm::vec4(0.0f,1.0f,0.0f,1.0f);

private:

  int mNumLines;

  GLuint mVertexBuffer;

  GLuint mVertexArrayId;
  // TODO: Share programs
  GLuint mProgramId;
  GLuint mViewProjMatrixId;
  GLuint mColorId;


  glm::mat4 mModelMatrix;
  glm::vec3 mPosition = glm::vec3(0,0,5);

};

