#include "DensityField.h"
#include <math.h>

// Returns the indices in to the grid array corresponding to a given pos
void DensityField::findGridLocation(glm::vec3 pos, int& x, int& y, int& z) {
	 x = (int)(pos[0] / GRID_W);
	 y = (int)(pos[1] / GRID_H);
	 z = (int)(pos[2] / GRID_D);
}

// Record a stamp at given grid indices
void DensityField::stamp(int x, int y, int z, int expansion, float maxStamp) {
	for (int xi = x-expansion; xi < x+expansion + 1 ; xi++) {
		if (xi < 0 || xi >= GRID_DIM) {
			continue;
		}
		for (int yi = y - expansion; yi < y + expansion + 1; yi++) {
			if (yi < 0 || yi >= GRID_DIM) {
				continue;
			}
			for (int zi = z - expansion; zi < z + expansion + 1; zi++) {
				if (zi < 0 || zi >= GRID_DIM) {
					continue;
				}
				float stampRaw = maxStamp - (float)(pow(abs(x - xi) * 1.2, 1.5) + pow(abs(y - yi) * 1.2, 1.5) + pow(abs(z - zi) * 1.2, 1.5));
				float stamp = fmax(0, stampRaw);
				grid[xi][yi][zi] += stamp;
			}
		}
	}
}

void DensityField::recordParticleAt(glm::vec3 pos) {
	int x, y, z;
	findGridLocation(pos, x, y, z);
	stamp(x, y, z);
}

std::vector<DensityField::Entry> DensityField::profile(float threshold) {
	std::vector<Entry> profileLocations;
	for (int yi = 0; yi < GRID_DIM; yi++) {
		for (int zi = 0; zi < GRID_DIM; zi++) {
			bool start = false;
			float value = 0.0f;
			for (int xi = 0; xi < GRID_DIM; xi++) {
				if (start == false && grid[xi][yi][zi] > threshold) {
					start = true;
					profileLocations.push_back(Entry(xi, yi, zi));
					continue;
				}

				if (start == true && grid[xi][yi][zi] <= threshold) {
					start = false;
					profileLocations.push_back(Entry(xi, yi, zi));
					continue;
				}
			}
		}
	}
	return profileLocations;
}

double DensityField::evaluate(glm::vec3 pos) {
	return 0.0f;
}