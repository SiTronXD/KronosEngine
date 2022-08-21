#pragma once

#include "../Graphics/MeshData.h"

struct Plane
{
	glm::vec3 pos;
	glm::vec3 normal;

	Plane(const glm::vec3& pos, const glm::vec3& normal)
	{
		this->pos = pos;
		this->normal = glm::normalize(normal);
	}
};

class BSP
{
private:
	float projectPointOnNormal(const Vertex& v, const Plane& plane);

	bool inSameHalfSpace(const float& t0, const float& t1);

public:
	BSP();
	~BSP();

	void createFromMeshData(MeshData& meshData);
};