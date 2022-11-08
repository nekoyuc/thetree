#pragma once
#include <glm/glm.hpp>
#include "SketchField.h"
#include "DensityField.h"

#define MAX_PARTICLES 100000

class ParticleSystem {
public:
    ParticleSystem(DensityField* df = nullptr);
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

    void update(double delta, Field* field);
    void render();
    void init();

protected:

    DensityField* mDensityField;

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

    Particle mParticles[MAX_PARTICLES];
    int mLastUsedParticle;
    int mParticlesCount;

    int findUnusedParticle();
    void sortParticles();
};
