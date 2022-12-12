#pragma once
#include <glm/glm.hpp>
#include "Field.h"
#include "DensityGrid.h"
#include "LineRenderer.h"
#include <functional>
#include "TunableParameters.h"


class ParticleSystem {
public:
    ParticleSystem(DensityGrid* df = nullptr);
    ~ParticleSystem();

    struct Particle { // inner class
      glm::vec3 pos, speed;
      unsigned char r,g,b,a;
      float size, angle, weight;
      float life;
      float cameraDistance;

      glm::vec3 history[PARTICLE_HISTORY_LENGTH];
      int currentHistoryPosition = 0;

      void recordHistory(const glm::vec3& position);

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

    // for adding new particles
    bool addParticle = false;
    glm::vec3 newPos = glm::vec3(0.0f, 0.0f, 0.0f);

    //
    bool showTrail = true;

    // for erase effect
    bool eraseOn = false;
    glm::vec3 eraseRay = glm::vec3(0.0f, 0.0f, -1.0f);

protected:

    DensityGrid* mDensityGrid;

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

    float mCrossProductLength;

    int findUnusedParticle();
    void sortParticles();
};
