#include "DensityViz.h"
#include "helpers/controls.hpp"

#include <glm/glm.hpp>
#include <stdio.h>

void DensityViz::visualizeField(const std::vector<DensityGrid::Entry>& entries, const glm::mat4& viewMatrix) {
	for (int i = 0; i < mMaxParticles; i++) {
		mParticles[i].life = -1.0f;
	}
	printf("Starting 1\n");
	for (const auto& entry : entries) {
        int particleIndex = findUnusedParticle();
        mParticles[particleIndex].life = 100.0f; // has to be non zero        
		mParticles[particleIndex].pos = glm::vec3(entry.x, entry.y, entry.z);
 
        // Very bad way to generate a random color
        mParticles[particleIndex].r = (unsigned char)20 + entry.x * 100;
		mParticles[particleIndex].g = (unsigned char)30 + entry.y * 100;
        mParticles[particleIndex].b = (unsigned char)50 + entry.z * 100;
		mParticles[particleIndex].a = (unsigned char)180;

		mParticles[particleIndex].size = 0.01;
	}
	printf("Starting 2\n");
	mParticlesCount = 0;
	for (int i = 0; i < mMaxParticles; i++) {
		Particle& p = mParticles[i]; // shortcut
		if (p.life > 0.0f) {
			// Get camera position from viewMatrix
			glm::vec3 cameraPosition = glm::vec3(glm::inverse(viewMatrix)[3]);
			p.cameraDistance = glm::length(p.pos - cameraPosition);
			mParticlePositionSizeData[4 * mParticlesCount + 0] = p.pos.x;
			mParticlePositionSizeData[4 * mParticlesCount + 1] = p.pos.y;
			mParticlePositionSizeData[4 * mParticlesCount + 2] = p.pos.z;
			mParticlePositionSizeData[4 * mParticlesCount + 3] = p.size;
			mParticleColorData[4 * mParticlesCount + 0] = p.r;
			mParticleColorData[4 * mParticlesCount + 1] = p.g;
			mParticleColorData[4 * mParticlesCount + 2] = p.b;
			mParticleColorData[4 * mParticlesCount + 3] = p.a;
			mParticlesCount++;
		} else {
			p.cameraDistance = -1.0f;
		}
	}
	printf("Starting 3\n");
	sortParticles();

	glBindBuffer(GL_ARRAY_BUFFER, mParticlesPositionBuffer);
	glBufferData(GL_ARRAY_BUFFER, mMaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, mParticlesCount * sizeof(GLfloat) * 4, mParticlePositionSizeData);

	glBindBuffer(GL_ARRAY_BUFFER, mParticlesColorBuffer);
	glBufferData(GL_ARRAY_BUFFER, mMaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
	glBufferSubData(GL_ARRAY_BUFFER, 0, mParticlesCount * sizeof(GLubyte) * 4, mParticleColorData);

}