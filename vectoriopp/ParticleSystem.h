#pragma once
#include <glm/glm.hpp>

#define MAX_PARTICLES 100000

class ParticleSystem {
public:
    ParticleSystem() = default;
    ~ParticleSystem();

    struct Particle {
      glm::vec3 pos, speed;
      unsigned char r,g,b,a;
      float size, angle, weight;
      float life;
      float cameraDistance;
      bool operator<(const Particle& that) const {
  	return this->cameraDistance > that.cameraDistance;
      }
    };

    void init();
    void update(double delta);
    void render();

private:
    Particle mParticles[MAX_PARTICLES];
    int mLastUsedParticle;
    int mParticlesCount;

    GLuint mVertexArrayId;
    GLuint mProgramId;
    GLuint mCameraRightWorldspaceId;
    GLuint mCameraUpWorldspaceId;
    GLuint mViewProjMatrixId;
    GLuint mTextureId;
    GLuint mTexture;
    GLfloat mParticlePositionSizeData[MAX_PARTICLES*4];
    GLubyte mParticleColorData[MAX_PARTICLES*4];
    GLuint mBillboardVertexBuffer;
    GLuint mParticlesPositionBuffer;
    GLuint mParticlesColorBuffer;

    int findUnusedParticle();
    void sortParticles();
};
