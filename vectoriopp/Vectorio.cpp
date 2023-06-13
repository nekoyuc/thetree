#include "Vectorio.h"

#include <GL/glew.h>
#include "ColoredTriangles.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>

#include "DensityGrid.h"
#include "DensityViz.h"
#include "Field.h"
#include "LineRenderer.h"
#include "ParticleSystem.h"

using namespace glm;

GLfloat plane_data[] = {
-3.0f, -3.0f, 0.0f,
3.0f, -3.0f, 0.0f,
3.0f,  3.0f, 0.0f,
-3.0f, -3.0f, 0.0f,
3.0f, 3.0f, 0.0f,
-3.0f, 3.0f, 0.0f
};

struct Plane : public ColoredTriangles {
      glm::mat4 mModelMatrix = glm::mat4(1.0f);
      Plane() : ColoredTriangles(&plane_data[0], 18) {
          mModelMatrix = glm::rotate(mModelMatrix, (float)(M_PI / 2.0f), glm::vec3(1, 0, 0));
          mColor = glm::vec4(0.2, 0.2, 0.2, 0.97);
      }
      glm::mat4 getMVPMatrix() override {
          return mPVMatrix * mModelMatrix;
      }

      glm::mat4 mPVMatrix;
};

Vectorio::Vectorio() {
    mBasePlane = new Plane();
    mDrawPlane = new CameraFacingTriangles(&plane_data[0], 18);
    mDrawPlane->mPosition = glm::vec3(0.0f, 0.0f, -3.0f);
    mDrawPlane->mColor = glm::vec4(0.9, 0.9, 0.9, 0.4);
    mLineRenderer = new LineRenderer();
    mLineRenderer->addLine(0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    mLineRenderer->addLine(0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    mLineRenderer->addLine(0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f);

    mDensityGrid = new DensityGrid();
    mParticleSystem = new ParticleSystem(mDensityGrid);
    mLineField = new LineField();
    mDensityVisualizer = new DensityViz();
}

Vectorio::~Vectorio() {
    delete mBasePlane;
    delete mDrawPlane;
    delete mLineRenderer;
    delete mLineField;
    delete mParticleSystem;
    delete mDensityGrid;
    delete mDensityVisualizer;
}

void Vectorio::update(double delta) {
    mParticleSystem->update(delta, mLineField);
}

void Vectorio::render(const glm::mat4& projectionViewMatrix) {
    // Light blue background
    glClearColor(0.7f, 0.8f, 0.9f, 0.0f);
  
    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);


    mBasePlane->mPVMatrix = projectionViewMatrix;
    mBasePlane->render();
    if (mRenderDrawPlane) {
        mDrawPlane->render();
    }

    glDisable(GL_DEPTH_TEST);

    if (mHasDensityVisualization) {
        mDensityVisualizer->render();
    }

    glDisable(GL_BLEND);
    mParticleSystem->render();
    
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    mLineRenderer->render();
}

float Vectorio::getDrawPlaneDistance() {
    return mDrawPlane->mPosition.z;
}

void Vectorio::moveDrawPlane(const glm::vec3& diff) {
    mDrawPlane->mPosition += diff;
}

void Vectorio::addLine(float x1, float y1, float z1, float x2, float y2, float z2) {
    mLineRenderer->addLine(x1, y1, z1, x2, y2, z2);
}

void Vectorio::recordLine(float x1, float y1, float z1, float x2, float y2, float z2) {
    mLineField->recordLine(x1, y1, z1, x2, y2, z2);
}

void Vectorio::setEraseRay(const glm::vec3& ray) {
    mParticleSystem->eraseRay = ray;
}

void Vectorio::addParticle(const glm::vec3& ray) {
    mParticleSystem->newPos = ray;
    mParticleSystem->addParticle = true;
}

void Vectorio::stopAddParticle() {
    mParticleSystem->addParticle = false;
}

void Vectorio::toggleShowTrails() {
    mParticleSystem->showTrail = !mParticleSystem->showTrail;
}

void Vectorio::changeProfileThreshold(float diff) {
    mDensityGrid->threshold += diff;
    printf("threshold is %f\n", mDensityGrid->threshold);
}

std::future<std::vector<DensityGrid::Entry>> Vectorio::profile() {
    printf("Profiling \n");
    return mDensityGrid->profile();
}

void Vectorio::visualizeField(const std::vector<DensityGrid::Entry>& profile) {
    mDensityVisualizer->visualizeField(profile);
    mHasDensityVisualization = true;
}

void Vectorio::setEraseOn(bool on) {
    mParticleSystem->eraseOn = on;
}

void Vectorio::invertLineField() {
    mLineField->rotation_direction *= -1.0f;
    printf("\n\ncurrent rotation direction is %f", mLineField->rotation_direction);    
}
