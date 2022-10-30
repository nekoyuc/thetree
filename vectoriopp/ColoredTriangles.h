#pragma once

class ColoredTriangles {
public:
  ColoredTriangles(GLfloat* vertexBufferData, int numVertices);
  ~ColoredTriangles();
  void render();
private:
  GLfloat* mVertexBufferData;
  int mNumVertices;

  GLuint mVertexArrayId;
  GLuint mProgramId;
  GLuint mVertexBuffer;
  GLuint mViewProjMatrixId;
};


