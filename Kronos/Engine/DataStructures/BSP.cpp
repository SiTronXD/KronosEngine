#include "BSP.h"
#include "../Dev/Log.h"

#define LOOP_IND(index) ((index) % 3)

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
	const glm::vec3 debugColor = glm::vec3(1.0f, 1.0f, 1.0f);

	/*Plane plane(
		glm::vec3(0.35f, 0.0f, 0.35f),
		glm::vec3(1.0f, 0.0f, 1.0f)
	);*/
	/*Plane plane(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 1.0f)
	);*/
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
			vertices[triIndices[0]].color = debugColor;
		if (projT[1] == 0.0f)
			vertices[triIndices[1]].color = debugColor;
		if (projT[2] == 0.0f)
			vertices[triIndices[2]].color = debugColor;

		// All points are in the same half-space
		if ((projT[0] >= 0 && projT[1] >= 0 && projT[2] >= 0) ||
			(projT[0] <= 0 && projT[1] <= 0 && projT[2] <= 0))
		{
			newIndices.push_back(triIndices[0]);
			newIndices.push_back(triIndices[1]);
			newIndices.push_back(triIndices[2]);

			continue;
		}

		// Create max 2 new vertices
		uint32_t numNewVerts = 0;
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
				// Points
				Vertex& v0 = vertices[triIndices[i]];
				Vertex& v1 = vertices[triIndices[LOOP_IND(i + 2)]];

				// Interpolate new vertex
				float t = currentResult / (currentResult - lastResult);
				Vertex newVert = Vertex::interpolateVertex(
					v0, 
					v1,
					t
				);
				newVert.color = debugColor;

				// Add new vertex
				vertices.push_back(newVert);
				numNewVerts++;
			}

			lastResult = currentResult;
		}

		// The most common case
		if (numNewVerts == 2)
		{
			uint32_t baseIndex =
				/*this->inSameHalfSpace(projT[1], projT[2]) * 0 +*/
				this->inSameHalfSpace(projT[0], projT[2]) * 1 +
				this->inSameHalfSpace(projT[0], projT[1]) * 2;

			// Reverse order if needed
			if (baseIndex == 2)
			{
				const Vertex tempVert = vertices[vertices.size() - 2];
				vertices[vertices.size() - 2] = vertices[vertices.size() - 1];
				vertices[vertices.size() - 1] = tempVert;
			}

			// Add 3 new triangles
			newIndices.push_back(triIndices[baseIndex]);
			newIndices.push_back(newTriIndices[1]);
			newIndices.push_back(newTriIndices[0]);

			newIndices.push_back(triIndices[LOOP_IND(baseIndex + 1)]);
			newIndices.push_back(newTriIndices[0]);
			newIndices.push_back(newTriIndices[1]);

			newIndices.push_back(triIndices[LOOP_IND(baseIndex + 1)]);
			newIndices.push_back(triIndices[LOOP_IND(baseIndex + 2)]);
			newIndices.push_back(newTriIndices[0]);
		}
		// One vertex lies exactly on the plane
		else if (numNewVerts == 1)
		{
			uint32_t baseIndex = 
				(projT[0] == 0.0f) * 0 +
				(projT[1] == 0.0f) * 1 + 
				(projT[2] == 0.0f) * 2;

			// Add 2 new triangles
			newIndices.push_back(triIndices[baseIndex]);
			newIndices.push_back(newTriIndices[0]);
			newIndices.push_back(triIndices[LOOP_IND(baseIndex + 2)]);

			newIndices.push_back(triIndices[baseIndex]);
			newIndices.push_back(triIndices[LOOP_IND(baseIndex + 1)]);
			newIndices.push_back(newTriIndices[0]);
		}
	}

	Log::write("Num verts: " + std::to_string(vertices.size()));
	Log::write("Num ind: " + std::to_string(newIndices.size()));

	// Apply 
	meshData.getVertices().assign(vertices.begin(), vertices.end());
	meshData.getIndices().assign(newIndices.begin(), newIndices.end());
}
