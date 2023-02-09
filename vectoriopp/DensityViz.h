#include "ParticleSystem.h"
#include "DensityGrid.h"

class DensityViz : public ParticleSystem {
public:
	DensityViz(int numParticles = 30000) : ParticleSystem(nullptr, numParticles) {}
	void visualizeField(const std::vector<DensityGrid::Entry>& entries);
};
