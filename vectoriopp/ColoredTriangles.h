#pragma once

class ColoredTriangles {
public:
  ColoredTriangles(GLfloat* vertexBufferData, int numElements);
  virtual ~ColoredTriangles();
  void render(const glm::mat4& projection, const glm::mat4& view);

  glm::vec4 mColor = glm::vec4(0.7, 0.8, 0.95, 0.3);
  glm::vec3 mPosition = glm::vec3(0, 0, 25);

private:
  GLfloat* mVertexBufferData;
  int mNumElements;


  GLuint mVertexArrayId;
  GLuint mProgramId;
  GLuint mVertexBuffer;
  GLuint mViewProjMatrixId;
  GLuint mColorId;

  glm::mat4 mModelMatrix;
};