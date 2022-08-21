#include "BSP.h"

bool BSP::isPositiveHalfSpace(const Vertex& v, const Plane& plane)
{
	glm::vec3 planeToVert = v.pos - plane.pos;
	float t = glm::dot(planeToVert, plane.normal);

	return t >= 0.0f;
}

BSP::BSP()
{
}

BSP::~BSP()
{
}

void BSP::createFromMeshData(MeshData& meshData)
{
	Plane plane(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);

	// Make a copy of the mesh data
	std::vector<Vertex> vertices = meshData.getVertices();
	std::vector<uint32_t> indices = meshData.getIndices();

	// Loop through each triangle
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		Vertex& v0 = vertices[indices[i + 0]];
		Vertex& v1 = vertices[indices[i + 1]];
		Vertex& v2 = vertices[indices[i + 2]];

		if (this->isPositiveHalfSpace(v0, plane))
		{
			v0.color.r = 0.5f;
			v0.color.g = 0.0f;
			v0.color.b = 0.0f;
		}
		else
		{
			v0.color.r = 0.0f;
			v0.color.g = 0.5f;
			v0.color.b = 0.0f;
		}

		if (this->isPositiveHalfSpace(v1, plane))
		{
			v1.color.r = 0.5f;
			v1.color.g = 0.0f;
			v1.color.b = 0.0f;
		}
		else
		{
			v1.color.r = 0.0f;
			v1.color.g = 0.5f;
			v1.color.b = 0.0f;
		}

		if (this->isPositiveHalfSpace(v2, plane))
		{
			v2.color.r = 0.5f;
			v2.color.g = 0.0f;
			v2.color.b = 0.0f;
		}
		else
		{
			v2.color.r = 0.0f;
			v2.color.g = 0.5f;
			v2.color.b = 0.0f;
		}
	}

	// Apply 
	meshData.getVertices().assign(vertices.begin(), vertices.end());
	meshData.getIndices().assign(indices.begin(), indices.end());
}
