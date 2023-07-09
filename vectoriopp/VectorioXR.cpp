#include "Vectorio.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <chrono>

Vectorio* sInstance = nullptr;
std::chrono::steady_clock::time_point sLastTime;
glm::vec3 lastLeftHandPos = glm::vec3(0.0f);

extern "C" {
void initVectorioXR() {
    sInstance = new Vectorio();
    printf("Vendor %s\n", glGetString(GL_VENDOR));    
}

void teardownVectorioXR() {
    delete sInstance;
}

void vectorioXRUpdate(double delta, float* projectionMatrix, float* viewMatrix, float* leftHandPosition,
                      float* leftHandRotation, float* rightHandPosition, float* rightHandRotation,
                      float leftClicked, float rightClicked) {
    // Get start time
    sLastTime = std::chrono::steady_clock::now();
    glm::mat4 projection = glm::make_mat4(projectionMatrix);
    glm::mat4 view = glm::make_mat4(viewMatrix);
    
    // Print time in ms since start time
    auto now = std::chrono::steady_clock::now();
    auto diff = now - sLastTime;
    printf("Left hand position: %f %f %f\n", leftHandPosition[0], leftHandPosition[1], leftHandPosition[2]);
    if (rightClicked) {
        glm::vec3 rightHand = glm::vec3(rightHandPosition[0], rightHandPosition[1], rightHandPosition[2]);
        // Multiply by view matrix to get world position
        //rightHand = glm::vec3(view * glm::vec4(rightHand, 1.0f));
        
        sInstance->addParticle(rightHand);
    } else {
        sInstance->stopAddParticle();
    }
    if (leftClicked) {
        sInstance->addLine(lastLeftHandPos[0], lastLeftHandPos[1], lastLeftHandPos[2],
                           leftHandPosition[0], leftHandPosition[1], leftHandPosition[2]);
    }
    lastLeftHandPos = glm::vec3(leftHandPosition[0], leftHandPosition[1], leftHandPosition[2]);
    sInstance->update(1/72.0, projection, view);    
}

void vectorioXRRender(float* projectionMatrix, float* viewMatrix) {
    glm::mat4 projection = glm::make_mat4(projectionMatrix);
    glm::mat4 view = glm::make_mat4(viewMatrix);
    sInstance->render(projection, view);
    // Print time since last time in ms
    auto now = std::chrono::steady_clock::now();
    auto diff = now - sLastTime;
    sLastTime = now;
}
}
