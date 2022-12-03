#include "ParticleSystem.h"
#include "DensityGrid.h"

class DensityViz : public ParticleSystem {
public:
	void visualizeField(const std::vector<DensityGrid::Entry>& entries);
};
