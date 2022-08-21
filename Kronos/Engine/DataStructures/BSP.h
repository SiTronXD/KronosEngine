#pragma once

#include "../Graphics/MeshData.h"

struct Plane
{
	glm::vec3 pos;
	glm::vec3 normal;

	Plane(const glm::vec3& pos, const glm::vec3& normal)
		: pos(pos), normal(normal)
	{}
};

class BSP
{
private:
	bool isPositiveHalfSpace(const Vertex& v, const Plane& plane);

public:
	BSP();
	~BSP();

	void createFromMeshData(MeshData& meshData);
};