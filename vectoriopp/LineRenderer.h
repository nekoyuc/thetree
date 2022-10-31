#pragma once
#include <GL/glew.h>

#include <glm/glm.hpp>

#define NUM_LINES 10000

class LineRenderer {
public:
  LineRenderer();
  virtual ~LineRenderer();
  LineRenderer(const LineRenderer& other) = delete;

  void addLine(float x1, float y1, float z1, float x2, float y2, float z2);

  void render();
  virtual glm::mat4 getMVPMatrix();
private:
  GLfloat mVertexBufferData[NUM_LINES*6];
  int mNumVertices = 0;

  GLuint mVertexArrayId;
  GLuint mProgramId;
  GLuint mVertexBuffer;
  GLuint mViewProjMatrixId;

  glm::mat4 mModelMatrix;
  glm::vec3 mPosition = glm::vec3(0,0,5);
};

