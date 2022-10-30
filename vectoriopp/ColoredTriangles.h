#pragma once

class ColoredTriangles {
public:
  ColoredTriangles(GLfloat* vertexBufferData, int numVertices);
  virtual ~ColoredTriangles();
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

class CameraFacingTriangles : public ColoredTriangles {
public:
  CameraFacingTriangles(GLfloat* vertexBufferData, int numVertices);
  ~CameraFacingTriangles() = default;
  glm::mat4 getMVPMatrix() override;
};


