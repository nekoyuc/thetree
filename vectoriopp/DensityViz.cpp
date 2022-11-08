#include "DensityViz.h"
#include "helpers/controls.hpp"

#include <glm/glm.hpp>
#include <stdio.h>

void DensityViz::visualizeField(const std::vector<DensityField::Entry>& entries) {
	for (const auto& entry : entries) {
        int particleIndex = findUnusedParticle();
        mParticles[particleIndex].life = 0.1f; // has to be non zero        
		mParticles[particleIndex].pos = glm::vec3(entry.x, entry.y, entry.z);
 
        // Very bad way to generate a random color
        mParticles[particleIndex].r = (unsigned char)255;
        mParticles[particleIndex].g = (unsigned char)0;
        mParticles[particleIndex].b = (unsigned char)0;
		mParticles[particleIndex].a = (unsigned char)255;

        mParticles[particleIndex].size = (rand() % 1000) / 10000.0f + 0.005f;
	}

	mParticlesCount = 0;
	for (int i = 0; i < MAX_PARTICLES; i++) {
		Particle& p = mParticles[i]; // shortcut
		if (p.life > 0.0f) {
			p.cameraDistance = glm::length(p.pos - getCameraPosition());
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
	sortParticles();
}