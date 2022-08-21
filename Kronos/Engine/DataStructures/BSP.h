#pragma once

#include "../Graphics/MeshData.h"

struct ProjectionResult
{
	bool inHalfSpace;
	float t;
};

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
	ProjectionResult projectPointOnNormal(const Vertex& v, const Plane& plane);

public:
	BSP();
	~BSP();

	void createFromMeshData(MeshData& meshData);
};