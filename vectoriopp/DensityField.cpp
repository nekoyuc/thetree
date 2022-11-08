#include "DensityField.h"
#include <math.h>

#define CHECK_COORD(c) if (abs(c) > ROOM_W/2.0f) { return; }
#define COORD2POS(c)  (int)((c + ROOM_W/2.0f) * (GRID_DIM/(float)ROOM_W))
// Returns the indices in to the grid array corresponding to a given pos
void DensityField::findGridLocation(glm::vec3 pos, int& x, int& y, int& z) {
	 CHECK_COORD(pos[0]); CHECK_COORD(pos[1]); CHECK_COORD(pos[2]);
	 x = COORD2POS(pos[0]);
	 y = COORD2POS(pos[1]);
	 z = COORD2POS(pos[2]);
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

#define G2P(c) (((c/((float)GRID_DIM))*ROOM_W)-ROOM_W/2.0f)
#define PUSH_LOC(x,y,z) profileLocations.push_back(Entry(G2P(xi),G2P(yi),G2P(zi)));
std::vector<DensityField::Entry> DensityField::profile(float threshold) {
	std::vector<Entry> profileLocations;
	for (int yi = 0; yi < GRID_DIM; yi++) {
		for (int zi = 0; zi < GRID_DIM; zi++) {
			bool start = false;
			float value = 0.0f;
			for (int xi = 0; xi < GRID_DIM; xi++) {
				if (start == false && grid[xi][yi][zi] > threshold) {
					start = true;
					PUSH_LOC(x, y, z);
					continue;
				}

				if (start == true && grid[xi][yi][zi] <= threshold) {
					start = false;
					PUSH_LOC(x, y, z);
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