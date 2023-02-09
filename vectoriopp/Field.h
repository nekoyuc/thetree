#pragma once
#include <glm/glm.hpp>
#include "TunableParameters.h"

class Field {
public:
	virtual ~Field() = default;
	virtual glm::vec3 sampleField(float x1, float y1, float z1) = 0;
};

class LineField : public Field {
public:
	LineField() = default;
	~LineField() = default;

	float rotation_direction = 1.0f;

	void recordLine(float x1, float y1, float z1, float x2, float y2, float z2);
	glm::vec3 sampleField(float x1, float y1, float z1);
private:
	float mVertices[60000];
	int mNumVertices = 0;
};