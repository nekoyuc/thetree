#include <stdio.h>
#include <stdlib.h>

#include <vector>
#include <algorithm>

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/norm.hpp>
using namespace glm;


#include "helpers/shader.hpp"
#include "helpers/texture.hpp"
#include "helpers/controls.hpp"

#include "ParticleSystem.h"
#include "Linerenderer.h"

#include "TunableParameters.h"


static const GLfloat g_vertex_buffer_data[] = { 
  -0.5f, -0.5f, 0.0f,
  0.5f, -0.5f, 0.0f,
  -0.5f,  0.5f, 0.0f,
  0.5f,  0.5f, 0.0f,
};


ParticleSystem::ParticleSystem(DensityGrid* densityGrid, int maxParticles) : mMaxParticles(maxParticles) {
	mDensityGrid = densityGrid;
	mTrailRenderer = new LineRenderer(mMaxParticles * PARTICLE_HISTORY_LENGTH * 6);
	mTrailRenderer->mColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
	mParticlePositionSizeData = (GLfloat*)malloc(sizeof(GLfloat) * (mMaxParticles * 4));
	mParticleColorData = (GLubyte*)malloc(sizeof(GLubyte) * (mMaxParticles * 4));
	mParticles = (Particle*)malloc(sizeof(Particle) * mMaxParticles);
	//mParticles = (Particle*) new Particle[mMaxParticles];
	init();
}

ParticleSystem::~ParticleSystem() {
  // Cleanup VBO and shader
  glDeleteBuffers(1, &mParticlesColorBuffer);
  glDeleteBuffers(1, &mParticlesPositionBuffer);
  glDeleteBuffers(1, &mBillboardVertexBuffer);
  glDeleteProgram(mProgramId);
  glDeleteTextures(1, &mTexture);
  glDeleteVertexArrays(1, &mVertexArrayId);
  free(mParticlePositionSizeData);
  free(mParticleColorData);
  //delete mParticles;
}

int ParticleSystem::findUnusedParticle(){
  for(int i=mLastUsedParticle; i<mMaxParticles; i++){
    if (mParticles[i].life < 0){
      mLastUsedParticle = i;
      return i;
    }
  }
  for(int i=0; i<mLastUsedParticle; i++){
    if (mParticles[i].life < 0){
      mLastUsedParticle = i;
      return i;
    }
  }
  return 0; // All particles are taken, override the first one
}

void ParticleSystem::sortParticles(){
	std::sort(&mParticles[0], &mParticles[mMaxParticles]);
}

void ParticleSystem::init() {
  glGenVertexArrays(1, &mVertexArrayId);
  glBindVertexArray(mVertexArrayId);

  // Create and compile our GLSL program from the shaders
  mProgramId = LoadShaders( "Particle.vertexshader", "Particle.fragmentshader" );

  mCameraRightWorldspaceId  = glGetUniformLocation(mProgramId, "CameraRight_worldspace");
  mCameraUpWorldspaceId  = glGetUniformLocation(mProgramId, "CameraUp_worldspace");
  mViewProjMatrixId = glGetUniformLocation(mProgramId, "VP");

  // fragment shader
  mTextureId  = glGetUniformLocation(mProgramId, "myTextureSampler");
	
  //static GLfloat* mParticlePositionSizeData = new GLfloat[mMaxParticles * 4];
  //static GLubyte* mParticleColorData = new GLubyte[mMaxParticles * 4];
  for(int i=0; i<mMaxParticles; i++){
    mParticles[i].life = -1.0f;
    mParticles[i].cameraDistance = -1.0f;
  }

  mTexture = loadDDS("particle.DDS");

  glGenBuffers(1, &mBillboardVertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mBillboardVertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

  // The VBO containing the positions and sizes of the particles
  glGenBuffers(1, &mParticlesPositionBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mParticlesPositionBuffer);
  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, mMaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

  // The VBO containing the colors of the particles
  glGenBuffers(1, &mParticlesColorBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, mParticlesColorBuffer);
  // Initialize with empty (NULL) buffer : it will be updated later, each frame.
  glBufferData(GL_ARRAY_BUFFER, mMaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
}

void ParticleSystem::Particle::recordHistory(const glm::vec3& position) {
	history[currentHistoryPosition % PARTICLE_HISTORY_LENGTH] = position;
	currentHistoryPosition++;
}

void ParticleSystem::update(double delta, Field* field) {
  glm::mat4 ProjectionMatrix = getProjectionMatrix();
  glm::mat4 ViewMatrix = getViewMatrix();

  // TODO: Could remove argument
  //delta = 0.08;
  // We will need the camera's position in order to sort the particles
  // w.r.t the camera's distance.
  // There should be a F() function in common/controls.cpp, 
  // but this works too.
  glm::vec3 CameraPosition(glm::inverse(ViewMatrix)[3]);

  glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;


  // Generate 10 new particule each millisecond,
  // but limit this to 16 ms (60 fps), or if you have 1 long frame (1sec),
  // newparticles will be huge and the next frame even longer.
  const int newParticles = 500;
  int newparticles = (int)(delta*newParticles); 
  if (newparticles > (int)(0.016f*newParticles))
    newparticles = (int)(0.008f*newParticles); // maximum of new particles is 4 per millisecond

  delta /= 5.0;

  // Create new particles
  if (addParticle == true) {
	  for (int i = 0; i < newparticles; i++) {
		  int particleIndex = findUnusedParticle();
		  mParticles[particleIndex].life = 8.0f; // This particle will live 8 seconds.
		  mParticles[particleIndex].pos = newPos;
		  glm::vec3 maindir = glm::vec3(0.0f, 0.5f, 0.0f);
		  // Very bad way to generate a random direction; 
		  // See for instance http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution instead,
		  // combined with some user-controlled parameters (main direction, spread, etc)
		  glm::vec3 randomdir = glm::vec3(
			  (rand() % 2000 - 1000.0f) / 1000.0f,
			  (rand() % 2000 - 1000.0f) / 1000.0f,
			  (rand() % 2000 - 1000.0f) / 1000.0f
		  );
		  mParticles[particleIndex].speed = (maindir + randomdir * INIT_SPREAD) / 2.0f;
		  // Very bad way to generate a random color
		  mParticles[particleIndex].r = rand() % 256;
		  mParticles[particleIndex].g = rand() % 256;
		  mParticles[particleIndex].b = rand() % 256;
		  //  mParticles[particleIndex].a = (rand() % 256) / 2;
		  mParticles[particleIndex].a = 255;
		  mParticles[particleIndex].size = (rand() % 1000) / 30000.0f + 0.001f;
	  }
  }

  // Simulate all particles
  mParticlesCount = 0;
  mTrailCount = 0;
  mTrailRenderer->mNumVertices = 0;
  for (int i = 0; i < mMaxParticles; i++) {
	  Particle& p = mParticles[i]; // shortcut
	  if (p.pos.y < 0.0f || p.pos.x > 3 || p.pos.z > 3 || p.pos.x < -3 || p.pos.z < -3) {
		  p.life = 0.0f;
	  }
	  
	  if (eraseOn == true) {
		  if (p.life <= 0.0f) {
			  p.cameraDistance = -1.0f;
			  continue;
		  }
	      glm:vec3 PoToCa = getCameraPosition() - p.pos;  
		  mCrossProductLength = glm::length(glm::vec3(
			  eraseRay[1] * PoToCa[2] - eraseRay[2] * PoToCa[1],
			  eraseRay[2] * PoToCa[0] - eraseRay[0] * PoToCa[2],
			  eraseRay[0] * PoToCa[1] - eraseRay[1] * PoToCa[0]
		  ));	  

		  // destamp
		  if (mCrossProductLength < ERASE_TOLERANCE) {
			  int trailLength = 0;
			  if (p.currentHistoryPosition >= PARTICLE_HISTORY_LENGTH) {
				  trailLength = PARTICLE_HISTORY_LENGTH;
			  }
			  else {
				  trailLength = p.currentHistoryPosition;
			  }
			  for (int i = 0; i < trailLength; i++){
				  //if (mDensityGrid->mDontRecord == false) {
				  {
					  int x, y, z;
					  mDensityGrid->findGridLocation(p.history[i], x, y, z);
					  mDensityGrid->stamp(x, y, z, -1.0f);
				  }
			  }
			  p.life = 0.0f;
		  }
	  }

	  if (p.life > 0.0f) {

		  // particles only keep moving above certain speed
		  if (glm::length(p.speed) > MIN_SPEED) {
			  p.speed = MAINTAIN_SCALE * p.speed
				  + ACCELERATION_SCALE * glm::vec3(0.0f, -9.81f, 0.0f)
				  + FIELD_SCALE * (field->sampleField(p.pos[0], p.pos[1], p.pos[2]));

			  p.pos += //ACCELERATION_SCALE * p.speed * (float)delta // Acceleration
				  p.speed * (float)delta;

			  // stamp
			  if (mDensityGrid != nullptr) {
				  //mDensityGrid->recordParticleAt(p.pos);
				  //if (mDensityGrid->mDontRecord == false) {
				  {
					  int x, y, z;
					  mDensityGrid->findGridLocation(p.pos, x, y, z);
					  mDensityGrid->stamp(x, y, z, 1.0f);
				  }
			  }
			  p.recordHistory(p.pos);
		  }

		  mParticlePositionSizeData[4 * mParticlesCount + 0] = p.pos.x;
		  mParticlePositionSizeData[4 * mParticlesCount + 1] = p.pos.y;
		  mParticlePositionSizeData[4 * mParticlesCount + 2] = p.pos.z;
	      mParticlePositionSizeData[4 * mParticlesCount + 3] = p.size;
		  mParticleColorData[4 * mParticlesCount + 0] = p.r;
		  mParticleColorData[4 * mParticlesCount + 1] = p.g;
		  mParticleColorData[4 * mParticlesCount + 2] = p.b;
		  mParticleColorData[4 * mParticlesCount + 3] = p.a;		  

		  p.cameraDistance = glm::length(p.pos - getCameraPosition());
			  // Fill the GPU buffer
			  

			  // Update trails. Trail count is total number of trail vertices, count is number for
			  // current particle
		  if (showTrail == true && p.currentHistoryPosition > 1) {
			  int count = 0;
			  p.iterateHistory([&](const glm::vec3& pos) {
			  if (count < 2) {
				  mTrailRenderer->mVertexBufferData[3 * mTrailCount + 0] = pos.x;
				  mTrailRenderer->mVertexBufferData[3 * mTrailCount + 1] = pos.y;
				  mTrailRenderer->mVertexBufferData[3 * mTrailCount + 2] = pos.z;
				  mTrailRenderer->mNumVertices += 3;
				  mTrailCount++;
				  count++;
			  }else {
				  mTrailRenderer->mVertexBufferData[3 * mTrailCount + 5] = pos.z;
				  mTrailRenderer->mVertexBufferData[3 * mTrailCount + 4] = pos.y;
				  mTrailRenderer->mVertexBufferData[3 * mTrailCount + 3] = pos.x;
				  mTrailRenderer->mVertexBufferData[3 * mTrailCount + 2] = mTrailRenderer->mVertexBufferData[3 * mTrailCount - 1];
				  mTrailRenderer->mVertexBufferData[3 * mTrailCount + 1] = mTrailRenderer->mVertexBufferData[3 * mTrailCount - 2];
				  mTrailRenderer->mVertexBufferData[3 * mTrailCount + 0] = mTrailRenderer->mVertexBufferData[3 * mTrailCount - 3];
				  mTrailRenderer->mNumVertices += 6;
				  mTrailCount += 2;
			  }
				  });
		  }
		  mParticlesCount++;
	  }
	  else {
		  // Particles that just died will be put at the end of the buffer in SortParticles();
		  p.cameraDistance = -1.0f;
	  }
  }
  
  sortParticles();
  glBindBuffer(GL_ARRAY_BUFFER, mParticlesPositionBuffer);
  glBufferData(GL_ARRAY_BUFFER, mMaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, mParticlesCount * sizeof(GLfloat) * 4, mParticlePositionSizeData);

  glBindBuffer(GL_ARRAY_BUFFER, mParticlesColorBuffer);
  glBufferData(GL_ARRAY_BUFFER, mMaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
  glBufferSubData(GL_ARRAY_BUFFER, 0, mParticlesCount * sizeof(GLubyte) * 4, mParticleColorData);

  mTrailRenderer->uploadToGPU();

}

void ParticleSystem::render() {
  glm::mat4 ProjectionMatrix = getProjectionMatrix();
  glm::mat4 ViewMatrix = getViewMatrix();
  glm::mat4 ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

  
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  // Use our shader
  glUseProgram(mProgramId);
  
  // Bind our texture in Texture Unit 0
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, mTexture);
  // Set our "myTextureSampler" sampler to use Texture Unit 0
  glUniform1i(mTextureId, 0);
  
  // Same as the billboards tutorial
  auto cameraPosition = getCameraPosition();
  auto cameraRight = glm::vec3(ViewMatrix[0][0],ViewMatrix[1][0],ViewMatrix[2][0]);
  auto cameraUp = glm::vec3(ViewMatrix[0][1],ViewMatrix[1][1], ViewMatrix[2][1]);
  glUniform3f(mCameraRightWorldspaceId, cameraRight[0], cameraRight[1], cameraRight[2]);
  glUniform3f(mCameraUpWorldspaceId, cameraUp[0], cameraUp[1], cameraUp[2]);
  
  glUniformMatrix4fv(mViewProjMatrixId, 1, GL_FALSE, &ViewProjectionMatrix[0][0]);
  
  // 1rst attribute buffer : vertices
  glEnableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, mBillboardVertexBuffer);
  glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
			);
  
  // 2nd attribute buffer : positions of particles' centers
  glEnableVertexAttribArray(1);
  glBindBuffer(GL_ARRAY_BUFFER, mParticlesPositionBuffer);
  glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : x + y + z + size => 4
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
			);
  
  // 3rd attribute buffer : particles' colors
  glEnableVertexAttribArray(2);
  glBindBuffer(GL_ARRAY_BUFFER, mParticlesColorBuffer);
  glVertexAttribPointer(
			2,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			4,                                // size : r + g + b + a => 4
			GL_UNSIGNED_BYTE,                 // type
			GL_TRUE,                          // normalized?    *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
			0,                                // stride
			(void*)0                          // array buffer offset
			);
  
  // These functions are specific to glDrawArrays*Instanced*.
  // The first parameter is the attribute buffer we're talking about.
  // The second parameter is the "rate at which generic vertex attributes advance when rendering multiple instances"
  // http://www.opengl.org/sdk/docs/man/xhtml/glVertexAttribDivisor.xml
  glVertexAttribDivisor(0, 0); // particles vertices : always reuse the same 4 vertices -> 0
  glVertexAttribDivisor(1, 1); // positions : one per quad (its center)                 -> 1
  glVertexAttribDivisor(2, 1); // color : one per quad                                  -> 1
  
  // Draw the particules !
  // This draws many times a small triangle_strip (which looks like a quad).
  // This is equivalent to :
  // for(i in mParticlesCount) : glDrawArrays(GL_TRIANGLE_STRIP, 0, 4), 
  // but faster.
  glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, mParticlesCount);
  
  glDisableVertexAttribArray(0);
  glDisableVertexAttribArray(1);
  glDisableVertexAttribArray(2);  

  mTrailRenderer->render();
}

