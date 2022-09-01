#include <map>
#include "BSPNode.h"
#include "../Dev/Log.h"

//#define RENDER_SEPARATE_NODE_COLORS
#define MESH_CONVEX_EPSILON 0.1f

#define LOOP_IND(index) ((index) % 3)

float BSPNode::projectPointOnNormal(const Vertex& v, const Plane& plane)
{
	return this->projectPointOnNormal(v.pos, plane);
}

float BSPNode::projectPointOnNormal(const glm::vec3& p, const Plane& plane)
{
	glm::vec3 planeToVert = p - plane.pos;
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

	/*if (l <= 0.0f)
	{
		Log::write(
			"normal: " + 
			std::to_string(outputUnnormalizedNormal.x) + ", " + 
			std::to_string(outputUnnormalizedNormal.y) + ", " + 
			std::to_string(outputUnnormalizedNormal.z));
	}*/

	return l <= 0.0f;
}

bool BSPNode::isMeshConvex(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices)
{
	glm::vec3 tempNormal(1.0f);
	Plane tempPlane(glm::vec3(0.0f), glm::vec3(1.0f));
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		// Ignore degenerate triangles
		if (this->isTriangleDegenerate(
			vertices,
			indices[i + 0],
			indices[i + 1],
			indices[i + 2],
			tempNormal))
		{
			continue;
		}

		// Normal and plane
		tempNormal = glm::normalize(tempNormal);
		tempPlane.pos = vertices[indices[i + 0]].pos;
		tempPlane.normal = tempNormal;

		// Loop through the other triangles
		for (size_t j = 0; j < indices.size(); j += 3)
		{
			// Don't evauluate itself
			if (i == j)
				continue;

			float projT[3] =
			{
				this->projectPointOnNormal(vertices[indices[j + 0]], tempPlane),
				this->projectPointOnNormal(vertices[indices[j + 1]], tempPlane),
				this->projectPointOnNormal(vertices[indices[j + 2]], tempPlane),
			};

			// One triangle is in front of plane, this mesh is not convex
			if (projT[0] > MESH_CONVEX_EPSILON || projT[1] > MESH_CONVEX_EPSILON || projT[2] > MESH_CONVEX_EPSILON)
				return false;
		}
	}

	return true;
}

bool BSPNode::foundTriangle(
	std::vector<Vertex>& vertices,
	std::vector<uint32_t>& indices,
	uint32_t& triStartIndex,
	glm::vec3& outputUnnormalizedNormal)
{
	float randomT = (float)rand() / RAND_MAX;
	triStartIndex =
		((uint32_t)(randomT * (indices.size() / 3 - 1)) * 3);

	uint32_t tempIndex = triStartIndex;
	bool triDeg = this->isTriangleDegenerate(
		vertices,
		indices[tempIndex + 0],
		indices[tempIndex + 1],
		indices[tempIndex + 2],
		outputUnnormalizedNormal
	);

	// Found degenerate triangle, pick another one
	while (triDeg)
	{
		tempIndex = (tempIndex + 3) % indices.size();

		// Only degenerate triangles
		if (tempIndex == triStartIndex)
		{
			Log::warning("BSP leaf is empty because of degenerate triangles");
			// this->nodeIndices.assign(indices.begin(), indices.end());

			return false;
		}

		triDeg = this->isTriangleDegenerate(
			vertices,
			indices[tempIndex + 0],
			indices[tempIndex + 1],
			indices[tempIndex + 2],
			outputUnnormalizedNormal
		);
	}
	triStartIndex = tempIndex;

	return true;
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

	// Pick an appropriate triangle if possible
	uint32_t triStartIndex = 0;
	glm::vec3 normal = glm::vec3(0.0f);
	if (!this->foundTriangle(vertices, indices, triStartIndex, normal))
		return;

	// Plane position
	this->nodePlane.pos = vertices[indices[triStartIndex]].pos;

	// Plane normal
	normal = glm::normalize(normal);
	this->nodePlane.normal = normal;

	/*switch (this->depthLevel % 3)
	{
	case 0:
		normal = glm::vec3(1.0f, 0.0f, 0.0f);
		break;
	case 1:
		normal = glm::vec3(0.0f, 1.0f, 0.0f);
		break;
	case 2:
		normal = glm::vec3(0.0f, 0.0f, 1.0f);
		break;
	}
	this->nodePlane.normal = normal;*/

	// Add the chosen triangle to this node, 
	// and remove it from evaluation
	for (uint32_t i = 0; i < 3; ++i)
		this->nodeIndices.push_back(indices[triStartIndex + i]);
	indices.erase(
		indices.begin() + triStartIndex,
		indices.begin() + triStartIndex + 3
	);

	// No more triangles in this node
	if (indices.size() <= 0)
		return;

	// Split into children if needed
	if(!this->isMeshConvex(vertices, indices))//if (this->depthLevel < 10)
	{
		this->negativeChild = new BSPNode(this->depthLevel + 1);
		this->positiveChild = new BSPNode(this->depthLevel + 1);
	}
	// This node is a leaf
	else 
	{
		// Add all triangles to this node and exit
		for (size_t i = 0; i < indices.size(); ++i)
			this->nodeIndices.push_back(indices[i]);

		return;
	}

	std::map<uint64_t, Vertex> createdVertices;

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

		#ifdef RENDER_SEPARATE_NODE_COLORS
			if (projT[0] == 0.0f)
				vertices[triIndices[0]].color = debugColor;
			if (projT[1] == 0.0f)
				vertices[triIndices[1]].color = debugColor;
			if (projT[2] == 0.0f)
				vertices[triIndices[2]].color = debugColor;
		#endif

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
				// Point indices
				uint32_t index0 = triIndices[i];
				uint32_t index1 = triIndices[LOOP_IND(i + 2)];
				if (index0 > index1)
				{
					const uint32_t tempIndex = index0;
					index0 = index1;
					index1 = tempIndex;
				}

				// Points
				Vertex& v0 = vertices[index0];
				Vertex& v1 = vertices[index1];

				// Interpolate new vertex
				float t = currentResult / (currentResult - lastResult);
				Vertex newVert = Vertex::interpolateVertex(
					v0,
					v1,
					t
				);

				#ifdef RENDER_SEPARATE_NODE_COLORS
					newVert.color = debugColor;
				#endif

				// Add new vertex
				uint64_t edgeIndex = (uint64_t(index0) << 32) | uint64_t(index1);
				createdVertices.insert(std::pair<uint64_t, Vertex>(edgeIndex, newVert));
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
				const uint32_t tempIndex = newTriIndices[0];
				newTriIndices[0] = newTriIndices[1];
				newTriIndices[1] = tempIndex;
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


	#ifdef RENDER_SEPARATE_NODE_COLORS
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
	#endif

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

void BSPNode::traverseBackToFront(std::vector<uint32_t>& outputIndices, const glm::vec3& camPos)
{
	// Leaf node
	if (!this->negativeChild && !this->positiveChild)
	{
		for (size_t i = 0; i < this->nodeIndices.size(); ++i)
			outputIndices.push_back(this->nodeIndices[i]);

		return;
	}

	BSPNode* firstNode = this->positiveChild;
	BSPNode* secondNode = this->negativeChild;
	if (this->projectPointOnNormal(camPos, this->nodePlane) > 0.0f)
	{
		firstNode = this->negativeChild;
		secondNode = this->positiveChild;
	}


	firstNode->traverseBackToFront(outputIndices, camPos);
	for (size_t i = 0; i < this->nodeIndices.size(); ++i)
		outputIndices.push_back(this->nodeIndices[i]);
	secondNode->traverseBackToFront(outputIndices, camPos);
}

void BSPNode::traverseFrontToBack(std::vector<uint32_t>& outputIndices, const glm::vec3& camPos)
{
	// Leaf node
	if (!this->negativeChild && !this->positiveChild)
	{
		for (size_t i = 0; i < this->nodeIndices.size(); ++i)
			outputIndices.push_back(this->nodeIndices[i]);

		return;
	}

	BSPNode* firstNode = this->negativeChild;
	BSPNode* secondNode = this->positiveChild;
	if (this->projectPointOnNormal(camPos, this->nodePlane) > 0.0f)
	{
		firstNode = this->positiveChild;
		secondNode = this->negativeChild;
	}


	firstNode->traverseFrontToBack(outputIndices, camPos);
	for (size_t i = 0; i < this->nodeIndices.size(); ++i)
		outputIndices.push_back(this->nodeIndices[i]);
	secondNode->traverseFrontToBack(outputIndices, camPos);
}

void BSPNode::getTreeDepth(uint32_t& value)
{
	if (!this->negativeChild && !this->positiveChild)
	{
		value = max(value, this->depthLevel);
		return;
	}

	this->negativeChild->getTreeDepth(value);
	this->positiveChild->getTreeDepth(value);
}
