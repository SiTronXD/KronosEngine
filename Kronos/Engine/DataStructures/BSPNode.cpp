#include "BSPNode.h"
#include "../Dev/Log.h"

#define LOOP_IND(index) ((index) % 3)

float BSPNode::projectPointOnNormal(const Vertex& v, const Plane& plane)
{
	glm::vec3 planeToVert = v.pos - plane.pos;
	float t = glm::dot(planeToVert, plane.normal);

	return t;
}

bool BSPNode::inSameHalfSpace(const float& t0, const float& t1)
{
	return (t0 <= 0 && t1 <= 0) || (t0 >= 0 && t1 >= 0);
}

bool BSPNode::isTriangleDegenerate(
	std::vector<Vertex>& vertices, 
	const uint32_t& index0, 
	const uint32_t& index1, 
	const uint32_t& index2, 
	glm::vec3& outputUnnormalizedNormal)
{
	const Vertex& v0 = vertices[index0];
	const Vertex& v1 = vertices[index1];
	const Vertex& v2 = vertices[index2];
	const glm::vec3 edge0 = v1.pos - v0.pos;
	const glm::vec3 edge1 = v2.pos - v0.pos;
	outputUnnormalizedNormal = glm::cross(edge0, edge1);
	float l = glm::dot(outputUnnormalizedNormal, outputUnnormalizedNormal);

	if (l <= 0.0f)
	{
		Log::write(
			"normal: " + 
			std::to_string(outputUnnormalizedNormal.x) + ", " + 
			std::to_string(outputUnnormalizedNormal.y) + ", " + 
			std::to_string(outputUnnormalizedNormal.z));
	}

	return l <= 0.0f;
}

BSPNode::BSPNode(const uint32_t& depthLevel)
	: nodePlane(glm::vec3(0.0f), glm::vec3(1.0f)),
	negativeChild(nullptr),
	positiveChild(nullptr),
	depthLevel(depthLevel)
{
	
}

BSPNode::~BSPNode()
{
	delete this->negativeChild;
	delete this->positiveChild;
	this->negativeChild = nullptr;
	this->positiveChild = nullptr;
}

void BSPNode::splitMesh(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	// No triangles in this node
	if (indices.size() <= 0)
		return;

	// Pick an appropriate triangle
	float randomT = (float) rand() / RAND_MAX;
	uint32_t randTriStartIndex =
		((uint32_t)(randomT * (indices.size() / 3 - 1)) * 3);

	uint32_t tempIndex = randTriStartIndex;
	glm::vec3 normal = glm::vec3(0.0f);
	bool triDeg = this->isTriangleDegenerate(
		vertices,
		indices[tempIndex + 0],
		indices[tempIndex + 1],
		indices[tempIndex + 2],
		normal
	);

	// Found degenerate triangle, pick another one
	while (triDeg)
	{
		tempIndex = (tempIndex + 3) % indices.size();

		// Only degenerate triangles
		if (tempIndex == randTriStartIndex)
		{
			Log::warning("BSP leaf contains degenerate triangles");
			this->nodeIndices.assign(indices.begin(), indices.end());

			return;
		}

		triDeg = this->isTriangleDegenerate(
			vertices,
			indices[tempIndex + 0],
			indices[tempIndex + 1],
			indices[tempIndex + 2],
			normal
		);
	}
	randTriStartIndex = tempIndex;


	// Plane position
	this->nodePlane.pos = vertices[indices[randTriStartIndex]].pos;

	// Plane normal
	normal = glm::normalize(normal);
	this->nodePlane.normal = normal;

	// Split into children if needed
	if (this->depthLevel < 10)
	{
		this->negativeChild = new BSPNode(this->depthLevel + 1);
		this->positiveChild = new BSPNode(this->depthLevel + 1);
	}

	// This node is a leaf
	if (!this->negativeChild || !this->positiveChild)
	{
		this->nodeIndices.assign(indices.begin(), indices.end());

		return;
	}


	const glm::vec3 debugColor = glm::vec3(1.0f, 1.0f, 1.0f);

	this->nodeIndices.reserve(indices.size());

	std::vector<uint32_t> positiveSpaceIndices;
	std::vector<uint32_t> negativeSpaceIndices;
	positiveSpaceIndices.reserve(indices.size());
	negativeSpaceIndices.reserve(indices.size());

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
			this->projectPointOnNormal(vertices[triIndices[0]], this->nodePlane),
			this->projectPointOnNormal(vertices[triIndices[1]], this->nodePlane),
			this->projectPointOnNormal(vertices[triIndices[2]], this->nodePlane),
		};

		if (projT[0] == 0.0f)
			vertices[triIndices[0]].color = debugColor;
		if (projT[1] == 0.0f)
			vertices[triIndices[1]].color = debugColor;
		if (projT[2] == 0.0f)
			vertices[triIndices[2]].color = debugColor;

		// All points lie on the plane
		if (projT[0] == 0.0f && projT[1] == 0.0f && projT[2] == 0.0f)
		{
			this->nodeIndices.push_back(triIndices[0]);
			this->nodeIndices.push_back(triIndices[1]);
			this->nodeIndices.push_back(triIndices[2]);

			continue;
		}
		// All points are in the same half-space
		else if (projT[0] >= 0 && projT[1] >= 0 && projT[2] >= 0)
		{
			positiveSpaceIndices.push_back(triIndices[0]);
			positiveSpaceIndices.push_back(triIndices[1]);
			positiveSpaceIndices.push_back(triIndices[2]);

			continue;
		}
		else if (projT[0] <= 0 && projT[1] <= 0 && projT[2] <= 0)
		{
			negativeSpaceIndices.push_back(triIndices[0]);
			negativeSpaceIndices.push_back(triIndices[1]);
			negativeSpaceIndices.push_back(triIndices[2]);

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

			std::vector<uint32_t>* firstSideIndices = &positiveSpaceIndices;
			std::vector<uint32_t>* secondSideIndices = &negativeSpaceIndices;
			if (projT[baseIndex] < 0.0f)
			{
				firstSideIndices = &negativeSpaceIndices;
				secondSideIndices = &positiveSpaceIndices;
			}

			// Add 3 new triangles
			firstSideIndices->push_back(triIndices[baseIndex]);
			firstSideIndices->push_back(newTriIndices[1]);
			firstSideIndices->push_back(newTriIndices[0]);

			secondSideIndices->push_back(triIndices[LOOP_IND(baseIndex + 1)]);
			secondSideIndices->push_back(newTriIndices[0]);
			secondSideIndices->push_back(newTriIndices[1]);

			secondSideIndices->push_back(triIndices[LOOP_IND(baseIndex + 1)]);
			secondSideIndices->push_back(triIndices[LOOP_IND(baseIndex + 2)]);
			secondSideIndices->push_back(newTriIndices[0]);
		}
		// One vertex lies exactly on the plane
		else if (numNewVerts == 1)
		{
			uint32_t baseIndex =
				(projT[0] == 0.0f) * 0 +
				(projT[1] == 0.0f) * 1 +
				(projT[2] == 0.0f) * 2;

			std::vector<uint32_t>* firstSideIndices = &positiveSpaceIndices;
			std::vector<uint32_t>* secondSideIndices = &negativeSpaceIndices;
			if (projT[LOOP_IND(baseIndex + 2)] < 0.0f)
			{
				firstSideIndices = &negativeSpaceIndices;
				secondSideIndices = &positiveSpaceIndices;
			}

			// Add 2 new triangles
			firstSideIndices->push_back(triIndices[baseIndex]);
			firstSideIndices->push_back(newTriIndices[0]);
			firstSideIndices->push_back(triIndices[LOOP_IND(baseIndex + 2)]);

			secondSideIndices->push_back(triIndices[baseIndex]);
			secondSideIndices->push_back(triIndices[LOOP_IND(baseIndex + 1)]);
			secondSideIndices->push_back(newTriIndices[0]);
		}
	}

	glm::vec3 randCol0 = glm::vec3(
		(float) rand() / RAND_MAX,
		(float) rand() / RAND_MAX,
		(float) rand() / RAND_MAX
	);
	glm::vec3 randCol1 = glm::vec3(
		(float)rand() / RAND_MAX,
		(float)rand() / RAND_MAX,
		(float)rand() / RAND_MAX
	);
	for (size_t i = 0; i < negativeSpaceIndices.size(); ++i)
	{
		Vertex& v = vertices[negativeSpaceIndices[i]];
		v.color = randCol0;
	}
	for (size_t i = 0; i < positiveSpaceIndices.size(); ++i)
	{
		Vertex& v = vertices[positiveSpaceIndices[i]];
		v.color = randCol1;
	}

	this->negativeChild->splitMesh(vertices, negativeSpaceIndices);
	this->positiveChild->splitMesh(vertices, positiveSpaceIndices);
}

void BSPNode::getMergedIndices(std::vector<uint32_t>& outputIndices)
{
	// Child node indices
	if (this->negativeChild)
		this->negativeChild->getMergedIndices(outputIndices);
	if (this->positiveChild)
		this->positiveChild->getMergedIndices(outputIndices);

	// This node indices
	for (size_t i = 0; i < this->nodeIndices.size(); ++i)
		outputIndices.push_back(this->nodeIndices[i]);
}
