#include "DensityGrid.h"
#include <math.h>
#include <future>
#include "TunableParameters.h"

#define CHECK_COORD(c) if (abs(c) > ROOM_W/2.0f) { return; }
// Make sure points are in room scope
#define COORD2GRID(c)  (int)((c + ROOM_W/2.0f) * (GRID_NUM/(float)ROOM_W))

// Returns the indices in to the grid array corresponding to a given pos
void DensityGrid::findGridLocation(glm::vec3 pos, int& x, int& y, int& z) {
	 CHECK_COORD(pos[0]); CHECK_COORD(pos[1]); CHECK_COORD(pos[2]);
	 x = COORD2GRID(pos[0]);
	 y = COORD2GRID(pos[1]);
	 z = COORD2GRID(pos[2]);
}

// Record a stamp at given grid indices
void DensityGrid::stamp(int x, int y, int z, float plusMinus, int spread, float maxStamp) {
	for (int xi = x-spread; xi < x+spread + 1 ; xi++) {
		if (xi < 0 || xi >= GRID_NUM) {
			continue;
		}
		for (int yi = y - spread; yi < y + spread + 1; yi++) {
			if (yi < 0 || yi >= GRID_NUM) {
				continue;
			}
			for (int zi = z - spread; zi < z + spread + 1; zi++) {
				if (zi < 0 || zi >= GRID_NUM) {
					continue;
				}
				float& density = grid[xi][yi][zi].density;
				glm::vec3& normal = grid[xi][yi][zi].normal;
				int& stamp_number = grid[xi][yi][zi].stamp_number;
				float stampRaw = maxStamp - (float)(pow(abs(x - xi) * 1.2, 1.5) + pow(abs(y - yi) * 1.2, 1.5) + pow(abs(z - zi) * 1.2, 1.5));
				float stamp = fmax(0, stampRaw);
				density += plusMinus * stamp;
				if (stamp_number == 0) {
					normal = glm::vec3(xi - x, yi - y, zi - z);
				}
				else {
					normal = (normal * (float)stamp_number + glm::vec3(xi - x, yi - y, zi - z)) / (float)(stamp_number + 1);
				}
				stamp_number += 1;
			}
		}
	}
}

/*
void DensityGrid::recordParticleAt(glm::vec3 pos) {
	if (mDontRecord == true) {
		return;
	}
	int x, y, z;
	findGridLocation(pos, x, y, z);
	stamp(x, y, z);
}

void DensityGrid::doneProfiling() {
	mDontRecord = false;
}
*/

#define G2C(g) (((g/((float)GRID_NUM))*ROOM_W)-ROOM_W/2.0f)
std::future<std::vector<DensityGrid::Entry>> DensityGrid::profile() {
	//mDontRecord = true;
	return std::async(std::launch::async, [&]() {
		int maxLocations = 100000;
		//int currLocations = 0;
		std::vector<Entry> profileLocations;

		system("rm dots.txt");
		const char* path = "dots.txt";
		int writeFd = open(path, O_CREAT | O_WRONLY);
		dprintf(writeFd, "X Y Z R G B XN YN ZN\n");

		for (int yi = 0; yi < GRID_NUM; yi++) {
			for (int zi = 0; zi < GRID_NUM; zi++) {
				bool start = false;
				float value = 0.0f;
				for (int xi = 0; xi < GRID_NUM; xi++) {
					if (start == false && grid[xi][yi][zi].density > THRESHOLD) {
						start = true;
						profileLocations.push_back(Entry(G2C(xi), G2C(yi), G2C(zi)));
						dprintf(writeFd, "%f %f %f %f %f %f\n",
							G2C(xi), G2C(yi), G2C(zi),
							//rand() % 100 / 100.0f, rand() % 100 / 100.0f, rand() % 100 / 100.0f,
							grid[xi][yi][zi].normal[0], grid[xi][yi][zi].normal[1], grid[xi][yi][zi].normal[2]);
						//printf("entering density value is %f\n", grid[xi][yi][zi]);
						//currLocations++;
						if (profileLocations.size() > maxLocations) {
							return profileLocations;
						}
						continue;
					}

					if (start == true && grid[xi][yi][zi].density <= THRESHOLD) {
						start = false;
						profileLocations.push_back(Entry(G2C(xi), G2C(yi), G2C(zi)));
						dprintf(writeFd, "%f %f %f %f %f %f\n",
							G2C(xi), G2C(yi), G2C(zi),
							//rand() % 100 / 100.0f, rand() % 100 / 100.0f, rand() % 100 / 100.0f,
							grid[xi][yi][zi].normal[0], grid[xi][yi][zi].normal[1], grid[xi][yi][zi].normal[2]);
						//printf("exiting density value is %f\n", grid[xi][yi][zi]);
						if (profileLocations.size() > maxLocations) {
							return profileLocations;
						}
						continue;
					if (start == true && grid[xi][yi][zi].density > THRESHOLD) {
						if (grid[xi][yi][zi + 1].density <= THRESHOLD && zi + 1 < GRID_NUM) {
							profileLocations.push_back(Entry(G2C(xi), G2C(yi), G2C(zi + 1)));
							dprintf(writeFd, "%f %f %f %f %f %f\n",
								G2C(xi), G2C(yi), G2C(zi + 1),
								grid[xi][yi][zi + 1].normal[0], grid[xi][yi][zi + 1].normal[1], grid[xi][yi][zi + 1].normal[2]);
							}
						else if (grid[xi][yi][zi - 1].density <= THRESHOLD && zi - 1 < GRID_NUM) {
							profileLocations.push_back(Entry(G2C(xi), G2C(yi), G2C(zi - 1)));
							dprintf(writeFd, "%f %f %f %f %f %f\n",
								G2C(xi), G2C(yi), G2C(zi - 1),
								grid[xi][yi][zi - 1].normal[0], grid[xi][yi][zi - 1].normal[1], grid[xi][yi][zi - 1].normal[2]);
						}
						if (grid[xi][yi + 1][zi].density <= THRESHOLD && zi + 1 < GRID_NUM) {
							profileLocations.push_back(Entry(G2C(xi), G2C(yi + 1), G2C(zi)));
							dprintf(writeFd, "%f %f %f %f %f %f\n",
								G2C(xi), G2C(yi + 1), G2C(zi),
								grid[xi][yi + 1][zi].normal[0], grid[xi][yi + 1][zi].normal[1], grid[xi][yi + 1][zi].normal[2]);
						}
						else if (grid[xi][yi + 1][zi].density <= THRESHOLD && zi + 1 < GRID_NUM) {
							profileLocations.push_back(Entry(G2C(xi), G2C(yi + 1), G2C(zi)));
							dprintf(writeFd, "%f %f %f %f %f %f\n",
								G2C(xi), G2C(yi + 1), G2C(zi),
								grid[xi][yi + 1][zi].normal[0], grid[xi][yi + 1][zi].normal[1], grid[xi][yi + 1][zi].normal[2]);
						}
						}
					}
				}
			}
		}

		close(writeFd);

		printf("Profiling thread complete\n");
		return profileLocations;
		});
}

/*
double DensityGrid::evaluate(glm::vec3 pos) {
	return 0.0f;
}
*/