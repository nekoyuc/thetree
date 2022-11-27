#pragma once
#include <glm/glm.hpp>

class Field {
public:
	virtual ~Field() = default;
	virtual glm::vec3 sampleField(float x1, float y1, float z1) = 0;
};

class SketchField : public Field {
public:
	SketchField() = default;
	~SketchField() = default;

	void recordLine(float x1, float y1, float z1, float x2, float y2, float z2);
	glm::vec3 sampleField(float x1, float y1, float z1);
private:
	float mVertices[60000];
	int mNumVertices = 0;
};