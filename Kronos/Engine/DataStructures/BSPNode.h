#pragma once

#include <vector>
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

class BSPNode
{
private:
	std::vector<uint32_t> nodeIndices;

	Plane nodePlane;

	BSPNode* negativeChild;
	BSPNode* positiveChild;

	float projectPointOnNormal(const Vertex& v, const Plane& plane);

	bool inSameHalfSpace(const float& t0, const float& t1);

public:
	BSPNode(const uint32_t& depthLevel, const Plane& nodePlane);
	~BSPNode();

	void splitMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices);
	void getMergedIndices(std::vector<uint32_t>& outputIndices);
};