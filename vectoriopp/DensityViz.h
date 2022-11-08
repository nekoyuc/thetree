#include "ParticleSystem.h"
#include "DensityField.h"

class DensityViz : public ParticleSystem {
public:
	void visualizeField(const std::vector<DensityField::Entry>& entries);
};
