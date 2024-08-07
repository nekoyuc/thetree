#include "ParticleSystem.h"
#include "DensityGrid.h"

#include <glm/glm.hpp>

class DensityViz : public ParticleSystem {
public:
	DensityViz(int numParticles = 150000, int historyLen = 0) : ParticleSystem(nullptr, numParticles, historyLen) {}
	void visualizeField(const std::vector<DensityGrid::Entry>& entries, const glm::mat4& viewMatrix);
};
