#pragma once
#include <glm/glm.hpp>
#include "SketchField.h"
#include "DensityField.h"
#include "LineRenderer.h"
#include <functional>

#define MAX_PARTICLES 100000

#define PARTICLE_HISTORY_LENGTH 10

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

      glm::vec3 history[PARTICLE_HISTORY_LENGTH];
      int currentHistoryPosition = 0;

      void recordHistory(const glm::vec3& position) {
          history[currentHistoryPosition % PARTICLE_HISTORY_LENGTH] = position;
          currentHistoryPosition++;
      }

      void iterateHistory(const std::function<void(const glm::vec3&)>& run) {
          if (currentHistoryPosition < PARTICLE_HISTORY_LENGTH) {
              for (int i = 0; i < currentHistoryPosition; i++) {
                  run(history[i]);
              }
              return;
          }
          int start = (currentHistoryPosition % PARTICLE_HISTORY_LENGTH);
          for (int i = start; i < PARTICLE_HISTORY_LENGTH; i++) {
              run(history[i]);
          }
          for (int i = 0; i < start; i++) {
              run(history[i]);
          }
      }

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

    // Lines
    Particle mParticles[MAX_PARTICLES];
    int mLastUsedParticle;
    int mParticlesCount;

    LineRenderer* mTrailRenderer;
    int mTrailCount;

    int findUnusedParticle();
    void sortParticles();
};
