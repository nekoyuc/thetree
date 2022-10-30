#pragma once

class LineRenderer {
public:
  LineRenderer(GLfloat* vertexBufferData, int numVertices);
  virtual ~LineRenderer();
  void render();
  virtual glm::mat4 getMVPMatrix();
private:
  GLfloat* mVertexBufferData;
  int mNumVertices;

  GLuint mVertexArrayId;
  GLuint mProgramId;
  GLuint mVertexBuffer;
  GLuint mViewProjMatrixId;

  glm::mat4 mModelMatrix;
  glm::vec3 mPosition = glm::vec3(0,0,5);
};

