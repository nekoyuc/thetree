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

#include <chrono>

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
};

Vectorio::Vectorio() {
    mBasePlane = new Plane();
    mDrawPlane = new ColoredTriangles(&plane_data[0], 18);
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

void Vectorio::update(double delta, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix) {
    mParticleSystem->update(delta, mLineField, projectionMatrix, viewMatrix);
}

void Vectorio::render(const glm::mat4& projection, const glm::mat4& view) {
    // Get current time
    std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();
    // Light blue background
    glClearColor(0.7f, 0.8f, 0.9f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);


    mBasePlane->render(projection, view*mBasePlane->mModelMatrix);
    if (mRenderDrawPlane) {
        mDrawPlane->render(glm::translate(projection, mDrawPlane->mPosition),
            glm::identity<glm::mat4>());
    }

    glDisable(GL_DEPTH_TEST);

    if (mHasDensityVisualization) {
        mDensityVisualizer->render(projection, view);
    }

    glDisable(GL_BLEND);
    mParticleSystem->render(projection, view);
    
    glEnable(GL_BLEND);
// Control whether lines go in front of plane (commented means lines are always on top)
//    glEnable(GL_DEPTH_TEST);

    mLineRenderer->render(projection, view);
    // Print time in ms since start
    //    std::chrono::time_point<std::chrono::system_clock> end = std::chrono::system_clock::now();
    //    std::chrono::duration<double, std::milli> elapsed = end-start;
    //    printf("Render time: %f\n", elapsed.count());
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

void Vectorio::toggleShowTrails() {
    mParticleSystem->showTrail = !mParticleSystem->showTrail;
}

void Vectorio::addParticle(const glm::vec3& ray) {
    mParticleSystem->newPos = ray;
    mParticleSystem->addParticle = true;
}

void Vectorio::stopAddParticle() {
    mParticleSystem->addParticle = false;
}

std::future<std::vector<DensityGrid::Entry>> Vectorio::profile() {
    printf("Profiling \n");
    return mDensityGrid->profile();
}

void Vectorio::visualizeField(const std::vector<DensityGrid::Entry>& profile, const glm::mat4& view) {
    mDensityVisualizer->visualizeField(profile, view);
    mHasDensityVisualization = true;
}

void Vectorio::changeProfileThreshold(float diff) {
    mDensityGrid->threshold += diff;
    printf("threshold is %f\n", mDensityGrid->threshold);
}

void Vectorio::invertLineField() {
    mLineField->rotation_direction *= -1.0f;
    printf("\n\ncurrent rotation direction is %f", mLineField->rotation_direction);    
}

void Vectorio::setEraseRay(const glm::vec3& ray) {
    mParticleSystem->eraseRay = ray;
}

void Vectorio::setEraseOn(bool on) {
    mParticleSystem->eraseOn = on;
}