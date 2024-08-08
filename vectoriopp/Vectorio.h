#pragma once


#include <glm/glm.hpp>

#include "DensityGrid.h"
#include "ParticleSystem.h"

#include <future>
#include <vector>

/*
these are forward declarations. basically:
"These classes exist, but you don't need to know their full details right now.
Just trust me, you'll get the full definition later."
*/

class CameraFacingTriangles;
class ColoredTriangles;
class DensityGrid;
class DensityViz;
class Plane;
class LineRenderer;
class LineField;

class Vectorio {
public:
    Vectorio();
    ~Vectorio();

    void update(double delta, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
    void render(const glm::mat4& projection, const glm::mat4& view);

    float getDrawPlaneDistance();
    void moveDrawPlane(const glm::vec3& diff);
    void toggleRenderDrawPlane() { mRenderDrawPlane = !mRenderDrawPlane; }

    void addLine(float x1, float y1, float z1, float x2, float y2, float z2);
    void recordLine(float x1, float y1, float z1, float x2, float y2, float z2);
    void toggleShowTrails();

    void addParticle(const glm::vec3& pos);
    void stopAddParticle();
    
    std::future<std::vector<DensityGrid::Entry>> profile();
    void visualizeField(const std::vector<DensityGrid::Entry>& entries, const glm::mat4& viewMatrix);
    void changeProfileThreshold(float diff);
    void invertLineField();
    void clearFieldVisualizer();
    
    void setEraseRay(const glm::vec3& ray);
    void setEraseOn(bool eraseOn);

private:
    ColoredTriangles* mDrawPlane;
    DensityGrid* mDensityGrid;
    LineField* mLineField;
    LineRenderer* mLineRenderer;
    ParticleSystem* mParticleSystem;
    Plane* mBasePlane;
    DensityViz* mDensityVisualizer;

    bool mRenderDrawPlane = true;
    bool mHasDensityVisualization = false;
};
