#ifdef __cplusplus
extern "C" {
#endif    
    void initVectorioXR();
    void teardownVectorioXR();
    void vectorioXRUpdate(double delta, float* projectionMatrix, float* viewMatrix, float* leftHandPosition,
                          float* leftHandRotation, float* rightHandPosition, float* rightHandRotation,
                          float leftClicked, float rightClicked);
    void vectorioXRRender(float* projection, float* view);
#ifdef __cplusplus
}
#endif