#pragma once
#include <glm/glm.hpp>
#include "Field.h"
#include "DensityGrid.h"
#include "LineRenderer.h"
#include <functional>
#include "TunableParameters.h"


class ParticleSystem {
public:
    ParticleSystem(DensityGrid* df = nullptr, int maxParticles = 12000, int historyLen = PARTICLE_HISTORY_LENGTH);
    ~ParticleSystem();

    struct Particle { // inner class
      glm::vec3 pos, speed;
      unsigned char r,g,b,a;
      float size, weight;
      float life;
      float cameraDistance;
      int currentHistoryPosition = 0;

      glm::vec3 history[1]; //[PARTICLE_HISTORY_LENGTH];

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

    void update(double delta, Field* field, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
    void render(const glm::mat4& ProjectionMatrix, const glm::mat4& ViewMatrix);
    void init();

    // for adding new particles
    bool addParticle = false;
    glm::vec3 newPos = glm::vec3(0.0f, 0.0f, 0.0f);

    //
    bool showTrail = true;

    // for erase effect
    bool eraseOn = false;
    glm::vec3 eraseRay = glm::vec3(0.0f, 0.0f, -1.0f);

    size_t getParticleSize() { return sizeof(Particle) + sizeof(glm::vec3)*(mHistoryLength); }
    Particle& getParticle(int index) { 
        return *(Particle*)(reinterpret_cast<void*>(mParticles)+(getParticleSize()*index)); 
    }

protected:
    const int mMaxParticles;

    DensityGrid* mDensityGrid;

    GLuint mVertexArrayId;
    GLuint mProgramId;
    GLuint mCameraRightWorldspaceId;
    GLuint mCameraUpWorldspaceId;
    GLuint mViewProjMatrixId;
    GLuint mTimeId;
    GLuint mTextureId;
    GLuint mTimeEffectScaleId;
    GLuint mTexture;
    GLfloat* mParticlePositionSizeData;
    GLubyte* mParticleColorData;
    GLuint mBillboardVertexBuffer;
    GLuint mParticlesPositionBuffer;
    GLuint mParticlesColorBuffer;

    // Lines
    Particle* mParticles;
    int mLastUsedParticle = 0;
    int mParticlesCount;
    float mTimeEffectScale = 1.0f;

    int64_t mTime = 0;

    LineRenderer* mTrailRenderer;
    int mTrailCount;

    float mCrossProductLength;

    int mHistoryLength;

    int findUnusedParticle();
    void sortParticles();
};
