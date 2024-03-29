#pragma once


#include <glm/glm.hpp>

#include "DensityGrid.h"

#include <future>
#include <vector>

class CameraFacingTriangles;
class ColoredTriangles;
class DensityGrid;
class DensityViz;
class Plane;
class LineRenderer;
class ParticleSystem;
class LineField;

class Vectorio {
public:
    Vectorio();
    ~Vectorio();

    void update(double delta, const glm::mat4& projectionMatrix, const glm::mat4& viewMatrix);
    void render(const glm::mat4& projection, const glm::mat4& view);

    float getDrawPlaneDistance();
    void toggleRenderDrawPlane() { mRenderDrawPlane = !mRenderDrawPlane; }
    void toggleShowTrails();
    void invertLineField();

    void moveDrawPlane(const glm::vec3& diff);
    void changeProfileThreshold(float diff);
    std::future<std::vector<DensityGrid::Entry>> profile();
    void visualizeField(const std::vector<DensityGrid::Entry>& entries, const glm::mat4& viewMatrix);
    void clearFieldVisualizer();


    void addLine(float x1, float y1, float z1, float x2, float y2, float z2);
    void recordLine(float x1, float y1, float z1, float x2, float y2, float z2);
    void addParticle(const glm::vec3& pos);
    void stopAddParticle();

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