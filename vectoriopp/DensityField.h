#pragma once 
#include <glm/glm.hpp>
#include <GL/glew.h>
#include <vector>

#define GRID_DIM 576

class DensityField {
public:
	DensityField() = default;
	~DensityField() = default;

	struct Entry {
		float x, y, z;
		Entry(float xin, float yin, float zin) { x = xin; y = yin; z = zin; }
	};

	const float ROOM_W = 12.0f;
	const float ROOM_D = 6.0f;
	const float ROOM_H = 6.0f;
	
	const float GRID_W = ROOM_W / GRID_DIM;
	const float GRID_D = ROOM_D / GRID_DIM;
	const float GRID_H = ROOM_H / GRID_DIM;

    void recordParticleAt(glm::vec3 pos);
	double evaluate(glm::vec3 pos);
	std::vector<Entry> profile(float threshold = 250.0f);

private:
	GLfloat grid[GRID_DIM][GRID_DIM][GRID_DIM];
	void stamp(int grid_x, int grid_y, int grid_z, int expansion=3, float maxStamp = 14);
	void findGridLocation(glm::vec3 pos, int& x, int& y, int& z);
};

/*
class BodyField : public DensityField {
	BodyField(glm::vec3 pos);
	~BodyField();
};

class JointField : public DensityField {
	JointField(glm::vec3 pos);
	~JointField();
};
*/