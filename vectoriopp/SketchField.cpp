#include "SketchField.h"
#include <float.h>
#include <stdio.h>


void SketchField::recordLine(float x1, float y1, float z1, float x2, float y2, float z2) {
    if (mNumVertices + 6 > 60000 * 6) {
        printf("Out of space for lines \n");
        return;
    }

    mVertices[mNumVertices] = x1;
    mVertices[mNumVertices+1] = y1;
    mVertices[mNumVertices+2] = z1;
    mVertices[mNumVertices+3] = x2;
    mVertices[mNumVertices+4] = y2;
    mVertices[mNumVertices+5] = z2;

    mNumVertices += 6;
}

static glm::vec3 computeRotationField(glm::vec3 particlePos, glm::vec3 linePos, glm::vec3 direction) {
    auto p_to_l = linePos - particlePos;
    auto dot_pl_d = p_to_l[0] * direction[0] + p_to_l[1] * direction[1] + p_to_l[2] * direction[2];
    auto dot_d_d = direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2];

    float a = dot_pl_d / dot_d_d;
    float projection0 = direction[0] * a;
    float projection1 = direction[1] * a;
    float projection2 = direction[2] * a;

    auto rotation = glm::vec3((p_to_l[1] - projection1) * direction[2] - (p_to_l[2] - projection2) * direction[1],
         (p_to_l[2] - projection2) * direction[0] - (p_to_l[0] - projection0) * direction[2],
         (p_to_l[0] - projection0) * direction[1] - (p_to_l[1] - projection1) * direction[0]);

    return rotation;
}


#define MAX_DISTANCE 10.0f
#define ATTRACTOR_SCALE 0.1f
#define ROTATION_SCALE 1.0f
#define DRAG_SCALE 2.0f

glm::vec3 SketchField::sampleField(float x1, float y1, float z1) {
    double closestDistance = DBL_MAX;
    glm::vec3 cStart, cEnd;
    glm::vec3 result = { 0,0,0 };
    glm::vec3 samplePos = { x1, y1, z1 };
    for (int i = 0; i < mNumVertices; i += 6) {
        glm::vec3 lineStart = {
            mVertices[i],mVertices[i + 1],mVertices[i + 2] };
        glm::vec3 lineEnd = { mVertices[i + 3],mVertices[i + 4], mVertices[i + 5] };
        auto distance = glm::length((lineStart + lineEnd) * 0.5f - samplePos);
        if (distance < MAX_DISTANCE) {
            if (distance < closestDistance) {
                closestDistance = distance;
                cStart = lineStart;
                cEnd = lineEnd;
            }
        }
    }

    if (closestDistance < DBL_MAX) {
        glm::vec3 lineCenter = (cStart + cEnd) * 0.5f;
        glm::vec3 attraction = (lineCenter - samplePos);
        result += attraction*ATTRACTOR_SCALE
            + computeRotationField(samplePos, lineCenter, cEnd-cStart)*ROTATION_SCALE
            + DRAG_SCALE*(cEnd-cStart);
    }
    // no line center
    return result;
}