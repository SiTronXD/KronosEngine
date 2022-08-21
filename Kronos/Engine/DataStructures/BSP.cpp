#include "BSP.h"

ProjectionResult BSP::projectPointOnNormal(const Vertex& v, const Plane& plane)
{
	glm::vec3 planeToVert = v.pos - plane.pos;
	float t = glm::dot(planeToVert, plane.normal);

	return ProjectionResult
	{ 
		t >= 0.0f, 
		t 
	};
}

BSP::BSP()
{
}

BSP::~BSP()
{
}

void BSP::createFromMeshData(MeshData& meshData)
{
	/*
	Plane plane(
		glm::vec3(0.35f, 0.0f, 0.35f),
		glm::vec3(1.0f, 0.0f, 1.0f)
	);
	*/

	Plane plane(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);

	// Make a copy of the mesh data
	std::vector<Vertex> vertices = meshData.getVertices();
	std::vector<uint32_t> indices = meshData.getIndices();

	std::vector<uint32_t> newIndices;
	newIndices.reserve(indices.size());

	// Loop through each triangle
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		uint32_t triIndices[3] =
		{
			indices[i + 0],
			indices[i + 1],
			indices[i + 2]
		};
		ProjectionResult projResults[3] =
		{
			this->projectPointOnNormal(vertices[triIndices[0]], plane),
			this->projectPointOnNormal(vertices[triIndices[1]], plane),
			this->projectPointOnNormal(vertices[triIndices[2]], plane),
		};

		// All points are in the same half-space
		if (projResults[0].inHalfSpace == projResults[1].inHalfSpace &&
			projResults[1].inHalfSpace == projResults[2].inHalfSpace)
		{
			newIndices.push_back(triIndices[0]);
			newIndices.push_back(triIndices[1]);
			newIndices.push_back(triIndices[2]);

			continue;
		}

		// Create 2 vertices
		uint32_t newTriVertOffset = 0;
		Vertex newTriVertices[2]{};
		uint32_t newTriIndices[2] =
		{
			vertices.size(),
			vertices.size() + 1,
		};

		ProjectionResult lastResult = projResults[2];
		for (uint32_t i = 0; i < 3; ++i)
		{
			ProjectionResult currentResult = projResults[i];

			// Check if the points are in different half-spaces
			if (currentResult.inHalfSpace ^ lastResult.inHalfSpace)
			{
				Vertex& v0 = vertices[triIndices[i]];
				Vertex& v1 = vertices[triIndices[(i + 2) % 3]];

				float t = currentResult.t / (currentResult.t - lastResult.t);
				Vertex newVert = Vertex::interpolateVertex(
					v0, 
					v1,
					t
				);
				newVert.color = glm::vec3(1.0f, 1.0f, 1.0f);

				newTriVertices[newTriVertOffset] = newVert;
				newTriVertOffset++;
			}

			lastResult = currentResult;
		}

		// TODO: fix the general case
		vertices.push_back(newTriVertices[0]);
		vertices.push_back(newTriVertices[1]);

		newIndices.push_back(triIndices[0]);
		newIndices.push_back(newTriIndices[1]);
		newIndices.push_back(newTriIndices[0]);
		newIndices.push_back(triIndices[1]);
		newIndices.push_back(newTriIndices[0]);
		newIndices.push_back(newTriIndices[1]);
		newIndices.push_back(triIndices[1]);
		newIndices.push_back(triIndices[2]);
		newIndices.push_back(newTriIndices[0]);
	}

	// Apply 
	meshData.getVertices().assign(vertices.begin(), vertices.end());
	meshData.getIndices().assign(newIndices.begin(), newIndices.end());
}
