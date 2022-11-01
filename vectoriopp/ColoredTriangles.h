#pragma once

class ColoredTriangles {
public:
  ColoredTriangles(GLfloat* vertexBufferData, int numElements);
  virtual ~ColoredTriangles();
  void render();
  virtual glm::mat4 getMVPMatrix();

  glm::vec4 mColor = glm::vec4(0.0, 0.8, 0.2, 0.4);
private:
  GLfloat* mVertexBufferData;
  int mNumElements;

  GLuint mVertexArrayId;
  GLuint mProgramId;
  GLuint mVertexBuffer;
  GLuint mViewProjMatrixId;
  GLuint mColorId;

  glm::mat4 mModelMatrix;
  glm::vec3 mPosition = glm::vec3(0,0,5);
};

class CameraFacingTriangles : public ColoredTriangles {
public:
  CameraFacingTriangles(GLfloat* vertexBufferData, int numVertices);
  ~CameraFacingTriangles() = default;
  glm::mat4 getMVPMatrix() override;
};


