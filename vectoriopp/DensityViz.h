#include "ParticleSystem.h"
#include "DensityGrid.h"

#include <glm/glm.hpp>

class DensityViz : public ParticleSystem {
public:
	DensityViz(int numParticles = 30000) : ParticleSystem(nullptr, numParticles) {}
	void visualizeField(const std::vector<DensityGrid::Entry>& entries, const glm::mat4& viewMatrix);
};
