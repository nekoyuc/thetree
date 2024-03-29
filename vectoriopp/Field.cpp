#include "Field.h"
#include <float.h>
#include <stdio.h>


void LineField::recordLine(float x1, float y1, float z1, float x2, float y2, float z2) {
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

// rotation field
static glm::vec3 computeRotationField(glm::vec3 particlePos, glm::vec3 linePos, glm::vec3 direction) {
    auto p_to_l = linePos - particlePos;
    auto dot_pl_d = p_to_l[0] * direction[0] + p_to_l[1] * direction[1] + p_to_l[2] * direction[2];
    auto dot_d_d = direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2];

    float a = dot_d_d != 0 ? dot_pl_d / dot_d_d : 1;
    float projection0 = direction[0] * a;
    float projection1 = direction[1] * a;
    float projection2 = direction[2] * a;

    auto rotation = glm::vec3((p_to_l[1] - projection1) * direction[2] - (p_to_l[2] - projection2) * direction[1],
         (p_to_l[2] - projection2) * direction[0] - (p_to_l[0] - projection0) * direction[2],
         (p_to_l[0] - projection0) * direction[1] - (p_to_l[1] - projection1) * direction[0]);

    return rotation;
}



glm::vec3 LineField::sampleField(float x1, float y1, float z1) {
    double closestDistance = DBL_MAX;
    glm::vec3 closeStart, closeEnd;
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
                closeStart = lineStart;
                closeEnd = lineEnd;
            }
        }
    }

    if (closestDistance < DBL_MAX) {
        glm::vec3 lineCenter = (closeStart + closeEnd) * 0.5f;
        glm::vec3 attraction = (lineCenter - samplePos);
        result += attraction*ATTRACTOR_SCALE // * ((rand() % 4000) / 50000.0f + 0.99f)
            + computeRotationField(samplePos, lineCenter, rotation_direction * (closeEnd-closeStart))*ROTATION_SCALE //* ((rand() % 4000) / 50000.0f + 0.99f)
            + DRAG_SCALE*(closeEnd-closeStart);
    }
    // no line center
    return result;
}