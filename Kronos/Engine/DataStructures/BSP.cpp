#include "BSP.h"
#include "../Dev/Log.h"

float BSP::projectPointOnNormal(const Vertex& v, const Plane& plane)
{
	glm::vec3 planeToVert = v.pos - plane.pos;
	float t = glm::dot(planeToVert, plane.normal);

	return t;
}

bool BSP::inSameHalfSpace(const float& t0, const float& t1)
{
	return (t0 <= 0 && t1 <= 0) || (t0 >= 0 && t1 >= 0);
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
	/*
	Plane plane(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f)
	);
	*/
	Plane plane(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.1f, 1.0f)
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
		float projT[3] =
		{
			this->projectPointOnNormal(vertices[triIndices[0]], plane),
			this->projectPointOnNormal(vertices[triIndices[1]], plane),
			this->projectPointOnNormal(vertices[triIndices[2]], plane),
		};

		if (projT[0] == 0.0f)
			vertices[triIndices[0]].color = glm::vec3(1.0f, 1.0f, 1.0f);
		if (projT[1] == 0.0f)
			vertices[triIndices[1]].color = glm::vec3(1.0f, 1.0f, 1.0f);
		if (projT[2] == 0.0f)
			vertices[triIndices[2]].color = glm::vec3(1.0f, 1.0f, 1.0f);

		// All points are in the same half-space
		if ((projT[0] >= 0 && projT[1] >= 0 && projT[2] >= 0) ||
			(projT[0] <= 0 && projT[1] <= 0 && projT[2] <= 0))
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

		float lastResult = projT[2];
		for (uint32_t i = 0; i < 3; ++i)
		{
			float currentResult = projT[i];

			// Check if the points are in different half-spaces
			if (!this->inSameHalfSpace(currentResult, lastResult))
			{
				Vertex& v0 = vertices[triIndices[i]];
				Vertex& v1 = vertices[triIndices[(i + 2) % 3]];

				float t = currentResult / (currentResult - lastResult);
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


		if (newTriVertOffset == 2)
		{
			uint32_t loneVertIndex = 0;
			if (this->inSameHalfSpace(projT[0], projT[1]))
			{
				loneVertIndex = 2;
			}
			else if (this->inSameHalfSpace(projT[0], projT[2]))
			{
				loneVertIndex = 1;
			}

			// Reverse order if needed
			if (loneVertIndex != 2)
			{
				vertices.push_back(newTriVertices[0]);
				vertices.push_back(newTriVertices[1]);
			}
			else
			{
				vertices.push_back(newTriVertices[1]);
				vertices.push_back(newTriVertices[0]);
			}

			newIndices.push_back(triIndices[loneVertIndex]);
			newIndices.push_back(newTriIndices[1]);
			newIndices.push_back(newTriIndices[0]);
			newIndices.push_back(triIndices[(loneVertIndex + 1) % 3]);
			newIndices.push_back(newTriIndices[0]);
			newIndices.push_back(newTriIndices[1]);
			newIndices.push_back(triIndices[(loneVertIndex + 1) % 3]);
			newIndices.push_back(triIndices[(loneVertIndex + 2) % 3]);
			newIndices.push_back(newTriIndices[0]);
		}
		else if (newTriVertOffset == 1)
		{
			vertices.push_back(newTriVertices[0]);

			uint32_t loneVertIndex = 0;
			if (projT[1] == 0.0f)
				loneVertIndex = 1;
			else if(projT[2] == 0.0f)
				loneVertIndex = 2;

			newIndices.push_back(triIndices[loneVertIndex]);
			newIndices.push_back(newTriIndices[0]);
			newIndices.push_back(triIndices[(loneVertIndex + 2) % 3]);
			newIndices.push_back(triIndices[loneVertIndex]);
			newIndices.push_back(triIndices[(loneVertIndex + 1) % 3]);
			newIndices.push_back(newTriIndices[0]);
		}
	}

	// Apply 
	meshData.getVertices().assign(vertices.begin(), vertices.end());
	meshData.getIndices().assign(newIndices.begin(), newIndices.end());
}
