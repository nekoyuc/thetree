#pragma once 
#include <fcntl.h>
#include <unistd.h>

#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>
#include <future>
#include "TunableParameters.h"
#include <unordered_set>

struct DensityCell {
	float density = 0.0f;
	glm::vec3 normal = glm::vec3(0,0,0);
	int stamp_number = 0;
};

class DensityGrid {
public:
	DensityGrid() = default;
	~DensityGrid() = default;

	struct Entry {
		float x = 0, y = 0, z = 0;
		Entry(float xin, float yin, float zin) : x(xin), y(yin), z(zin) {}
	};

	const float ROOM_W = 12.0f;
	const float ROOM_D = 6.0f;
	const float ROOM_H = 6.0f;
	
	const float GRID_W = ROOM_W / GRID_NUM;
	const float GRID_D = ROOM_D / GRID_NUM;
	const float GRID_H = ROOM_H / GRID_NUM;

    //void recordParticleAt(glm::vec3 pos); // do stamp and findGridLocation
	//double evaluate(glm::vec3 pos);

	std::future<std::vector<Entry>> profile();
	//void doneProfiling(); // turn off recordParticleAt when true
	bool mDontRecord = false;

	void stamp(int grid_x, int grid_y, int grid_z, float plusMinus = 1.0f, int spread = STAMP_SPREAD, float maxStamp = MAX_STAMP);
	void findGridLocation(glm::vec3 pos, int& x, int& y, int& z);

private:
	DensityCell grid[GRID_NUM][GRID_NUM][GRID_NUM];
};

/*
class BodyField : public DensityGrid {
	BodyField(glm::vec3 pos);
	~BodyField();
};

class JointField : public DensityGrid {
	JointField(glm::vec3 pos);
	~JointField();
};
*/