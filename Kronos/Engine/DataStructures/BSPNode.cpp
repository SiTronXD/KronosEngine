#include "BSPNode.h"
#include "../Dev/Log.h"

//#define RENDER_SEPARATE_NODE_COLORS
#define EPSILON 0.0001f
#define LOWER_TREE_DEPTH

#define LOOP_IND(index) ((index) % 3)

void BSPNode::replaceEdgeForIndices(
	std::map<uint64_t, uint32_t>& createdVertIndex, 
	std::vector<uint32_t>& indicesOutput)
{
	std::vector<uint32_t> newIndices;
	newIndices.reserve(indicesOutput.size());
	for (size_t i = 0; i < indicesOutput.size(); i += 3)
	{
		uint32_t triIndices[3] =
		{
			indicesOutput[i + 0],
			indicesOutput[i + 1],
			indicesOutput[i + 2]
		};

		bool shouldReplaceEdge = false;

		// Go through each edge
		for (size_t j = 0; j < 3; ++j)
		{
			uint64_t edgeIndex = this->getEdgeIndex(
				triIndices[j],
				triIndices[LOOP_IND(j + 1)]
			);

			// Check if edge should be replaced
			if (createdVertIndex.count(edgeIndex))
			{
				shouldReplaceEdge = true;

				uint32_t newVertIndex = createdVertIndex[edgeIndex];

				newIndices.push_back(newVertIndex);
				newIndices.push_back(triIndices[LOOP_IND(j + 2)]);
				newIndices.push_back(triIndices[LOOP_IND(j + 0)]);

				newIndices.push_back(newVertIndex);
				newIndices.push_back(triIndices[LOOP_IND(j + 1)]);
				newIndices.push_back(triIndices[LOOP_IND(j + 2)]);
			}
		}

		if (!shouldReplaceEdge)
		{
			newIndices.push_back(triIndices[0]);
			newIndices.push_back(triIndices[1]);
			newIndices.push_back(triIndices[2]);
		}
	}
	indicesOutput.assign(newIndices.begin(), newIndices.end());
}

uint64_t BSPNode::getEdgeIndex(const uint32_t& index0, const uint32_t& index1)
{
	return (uint64_t(std::min(index0, index1)) << 32) | uint64_t(std::max(index0, index1));
}

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

bool BSPNode::isZero(const float& x)
{
	// if(abs(x - y) <= epsilon * max(abs(x), abs(y), 1.0f))
	return std::abs(x) <= EPSILON * std::max(std::abs(x), 1.0f);
}

bool BSPNode::isLargerThanZero(const float& x)
{
	//return x >= -EPSILON;
	
	return x >= -EPSILON * std::max(abs(x), 1.0f);
}

bool BSPNode::isLessThanZero(const float& x)
{
	//return x <= EPSILON;

	return x <= EPSILON * std::max(abs(x), 1.0f);
}

bool BSPNode::isLargerThanUpperZero(const float& x)
{
	return x > EPSILON * std::max(abs(x), 1.0f);
}

bool BSPNode::inSameHalfSpace(const float& t0, const float& t1)
{
	return (this->isLessThanZero(t0) && this->isLessThanZero(t1)) ||
		(this->isLargerThanZero(t0) && this->isLargerThanZero(t1));
	//return (t0 <= 0 && t1 <= 0) || (t0 >= 0 && t1 >= 0);
}

bool BSPNode::isTriangleDegenerate(std::vector<Vertex>& vertices, const uint32_t& index0, const uint32_t& index1, const uint32_t& index2)
{
	const Vertex& v0 = vertices[index0];
	const Vertex& v1 = vertices[index1];
	const Vertex& v2 = vertices[index2];
	const glm::vec3 edge0 = v1.pos - v0.pos;
	const glm::vec3 edge1 = v2.pos - v0.pos;
	glm::vec3 normal = glm::cross(edge0, edge1);
	float lengthSquared = glm::dot(normal, normal);

	/*if (l <= 0.0f)
	{
		Log::write(
			"normal: " +
			std::to_string(outputUnnormalizedNormal.x) + ", " +
			std::to_string(outputUnnormalizedNormal.y) + ", " +
			std::to_string(outputUnnormalizedNormal.z));
	}*/

	return lengthSquared <= 0.0f;
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
	float lengthSquared = glm::dot(outputUnnormalizedNormal, outputUnnormalizedNormal);

	/*if (l <= 0.0f)
	{
		Log::write(
			"normal: " +
			std::to_string(outputUnnormalizedNormal.x) + ", " +
			std::to_string(outputUnnormalizedNormal.y) + ", " +
			std::to_string(outputUnnormalizedNormal.z));
	}*/

	return lengthSquared <= 0.0f;
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
			/*if (projT[0] > MESH_CONVEX_EPSILON || projT[1] > MESH_CONVEX_EPSILON || projT[2] > MESH_CONVEX_EPSILON)
				return false;*/
			if (this->isLargerThanUpperZero(projT[0]) || 
				this->isLargerThanUpperZero(projT[1]) || 
				this->isLargerThanUpperZero(projT[2]))
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
#ifndef LOWER_TREE_DEPTH

	// Try a random triangle
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

#else

	// Try to find an optimal triangle
	triStartIndex = 0;
	uint32_t lowestDifference = ~0u;
	bool foundTriangle = false;
	for (uint32_t i = 0; i < indices.size(); i += 3)
	{
		glm::vec3 planeNormal;
		if (!this->isTriangleDegenerate(
			vertices,
			indices[i + 0],
			indices[i + 1],
			indices[i + 2],
			planeNormal))
		{
			foundTriangle = true;

			planeNormal = glm::normalize(planeNormal);

			Plane tempPlane(vertices[indices[i + 0]].pos, planeNormal);
			int32_t difference = 0;
			for (uint32_t j = 0; j < indices.size(); j += 3)
			{
				if (i == j)
					continue;

				float projT[3] =
				{
					this->projectPointOnNormal(vertices[indices[j + 0]], tempPlane),
					this->projectPointOnNormal(vertices[indices[j + 1]], tempPlane),
					this->projectPointOnNormal(vertices[indices[j + 2]], tempPlane),
				};

				if (!(this->isZero(projT[0]) && this->isZero(projT[1]) && this->isZero(projT[2])))
				{
					if (this->isLargerThanZero(projT[0]) && this->isLargerThanZero(projT[1]) && this->isLargerThanZero(projT[2]))
						difference++;
					else if (this->isLessThanZero(projT[0]) && this->isLessThanZero(projT[1]) && this->isLessThanZero(projT[2]))
						difference--;
				}
			}

			if (uint32_t(std::abs(difference)) < lowestDifference)
			{
				lowestDifference = uint32_t(std::abs(difference));
				triStartIndex = i;
				outputUnnormalizedNormal = planeNormal;
			}
		}
	}

	if (!foundTriangle)
	{
		Log::warning("BSP leaf is empty because of degenerate triangles");
		// this->nodeIndices.assign(indices.begin(), indices.end());
	}

	return foundTriangle;
#endif
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

void BSPNode::splitMesh(
	std::vector<Vertex>& vertices, 
	BSPNode* rootNode)
{
	// No triangles in this node
	if (this->tempNodeIndices.size() <= 0)
		return;

	// Pick an appropriate triangle if possible
	uint32_t triStartIndex = 0;
	glm::vec3 normal = glm::vec3(0.0f);
	if (!this->foundTriangle(vertices, this->tempNodeIndices, triStartIndex, normal))
		return;

	// Plane position
	this->nodePlane.pos = vertices[this->tempNodeIndices[triStartIndex]].pos;

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
		this->nodeIndices.push_back(this->tempNodeIndices[triStartIndex + i]);
	this->tempNodeIndices.erase(
		this->tempNodeIndices.begin() + triStartIndex,
		this->tempNodeIndices.begin() + triStartIndex + 3
	);

	// No more triangles in this node
	if (this->tempNodeIndices.size() <= 0)
		return;

	// Split into children if needed
	if (!this->isMeshConvex(vertices, this->tempNodeIndices))
	{
		this->negativeChild = new BSPNode(this->depthLevel + 1);
		this->positiveChild = new BSPNode(this->depthLevel + 1);
	}
	// This node is a leaf
	else
	{
		// Add all triangles to this node and exit
		for (size_t i = 0; i < this->tempNodeIndices.size(); ++i)
			this->nodeIndices.push_back(this->tempNodeIndices[i]);

		return;
	}

	std::map<uint64_t, uint32_t> createdVertIndex;

	const glm::vec3 debugColor = glm::vec3(1.0f, 1.0f, 1.0f);

	this->nodeIndices.reserve(this->tempNodeIndices.size());

	std::vector<uint32_t> positiveSpaceIndices;
	std::vector<uint32_t> negativeSpaceIndices;
	positiveSpaceIndices.reserve(this->tempNodeIndices.size());
	negativeSpaceIndices.reserve(this->tempNodeIndices.size());

	// Loop through each triangle
	for (size_t i = 0; i < this->tempNodeIndices.size(); i += 3)
	{
		uint32_t triIndices[3] =
		{
			this->tempNodeIndices[i + 0],
			this->tempNodeIndices[i + 1],
			this->tempNodeIndices[i + 2]
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
		if (this->isTriangleDegenerate(
			vertices,
			triIndices[0],
			triIndices[1],
			triIndices[2]) || 
			(this->isZero(projT[0]) && this->isZero(projT[1]) && this->isZero(projT[2])))
		{
			this->nodeIndices.push_back(triIndices[0]);
			this->nodeIndices.push_back(triIndices[1]);
			this->nodeIndices.push_back(triIndices[2]);

			continue;
		}
		// All points are in the same half-space
		else if (projT[0] >= 0.0f && projT[1] >= 0.0f && projT[2] >= 0.0f)
		{
			positiveSpaceIndices.push_back(triIndices[0]);
			positiveSpaceIndices.push_back(triIndices[1]);
			positiveSpaceIndices.push_back(triIndices[2]);

			continue;
		}
		else if (projT[0] <= 0.0f && projT[1] <= 0.0f && projT[2] <= 0.0f)
		{
			negativeSpaceIndices.push_back(triIndices[0]);
			negativeSpaceIndices.push_back(triIndices[1]);
			negativeSpaceIndices.push_back(triIndices[2]);

			continue;
		}
		// All points are in the same half-space
		else if (this->isLargerThanZero(projT[0]) && 
			this->isLargerThanZero(projT[1]) && 
			this->isLargerThanZero(projT[2]))
		{
			positiveSpaceIndices.push_back(triIndices[0]);
			positiveSpaceIndices.push_back(triIndices[1]);
			positiveSpaceIndices.push_back(triIndices[2]);

			continue;
		}
		else if (this->isLessThanZero(projT[0]) && 
			this->isLessThanZero(projT[1]) && 
			this->isLessThanZero(projT[2]))
		{
			negativeSpaceIndices.push_back(triIndices[0]);
			negativeSpaceIndices.push_back(triIndices[1]);
			negativeSpaceIndices.push_back(triIndices[2]);

			continue;
		}

		/*glm::vec3 triNormal;
		if (this->isTriangleDegenerate(
			vertices,
			triIndices[0],
			triIndices[1],
			triIndices[2],
			triNormal))
		{
			Log::error("Tries to clip degenerate triangle");
		}*/

		// Create max 2 new vertices
		uint32_t numNewVerts = 0;
		uint32_t newTriIndices[2] = { ~0u, ~0u };

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

				assert(index0 != index1);

				uint64_t edgeIndex = this->getEdgeIndex(index0, index1);
				if (!createdVertIndex.count(edgeIndex))
				{
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

					/*t = max(0.001f, t);
					t = min(0.999f, t);*/

					/*if (t <= 0.0001f || t >= 0.9999f)
					{
						Log::error("t");
					}*/

#ifdef RENDER_SEPARATE_NODE_COLORS
					newVert.color = debugColor;
#endif

					/*if (vertices.size() == 3646)
					{
						Log::warning("xd");
					}*/

					// Add new vertex
					newTriIndices[numNewVerts] = vertices.size();
					createdVertIndex.insert(std::pair<uint64_t, uint32_t>(edgeIndex, newTriIndices[numNewVerts]));
					vertices.push_back(newVert);
				}
				else
				{
					newTriIndices[numNewVerts] = createdVertIndex[edgeIndex];
				}
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
				(this->isZero(projT[0])) * 0 +
				(this->isZero(projT[1])) * 1 +
				(this->isZero(projT[2])) * 2;

			assert(baseIndex >= 0 && baseIndex <= 2);

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
		else if (numNewVerts == 0)
		{
			Log::error("No new vertices, despite expecting new ones.");
		}
	}


#ifdef RENDER_SEPARATE_NODE_COLORS
	glm::vec3 randCol0 = glm::vec3(
		(float)rand() / RAND_MAX,
		(float)rand() / RAND_MAX,
		(float)rand() / RAND_MAX
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

	this->negativeChild->assignSpaceIndices(negativeSpaceIndices);
	this->positiveChild->assignSpaceIndices(positiveSpaceIndices);

	// Traverse from root and replace shared edges
	rootNode->replaceEdges(createdVertIndex);

	this->negativeChild->splitMesh(vertices, rootNode);
	this->positiveChild->splitMesh(vertices, rootNode);


	this->tempNodeIndices.clear();
}

void BSPNode::replaceEdges(std::map<uint64_t, uint32_t>& createdVertIndex)
{
	this->replaceEdgeForIndices(createdVertIndex, this->nodeIndices);
	this->replaceEdgeForIndices(createdVertIndex, this->tempNodeIndices);

	// Continue traversal
	if (this->negativeChild && this->positiveChild)
	{
		this->negativeChild->replaceEdges(createdVertIndex);
		this->positiveChild->replaceEdges(createdVertIndex);
	}
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
	//if (this->isLargerThanZero(this->projectPointOnNormal(camPos, this->nodePlane)))
	if (this->projectPointOnNormal(camPos, this->nodePlane) > 0.0f)
	{
		firstNode = this->negativeChild;
		secondNode = this->positiveChild;
	}

	/*if (this->isZero(this->projectPointOnNormal(camPos, this->nodePlane)))
	{
		Log::write("CAM IN PLANE!!!!");
		firstNode->traverseBackToFront(outputIndices, camPos);
		for (size_t i = 0; i < this->nodeIndices.size(); ++i)
			outputIndices.push_back(0);
		secondNode->traverseBackToFront(outputIndices, camPos);
	}
	else
	{
		firstNode->traverseBackToFront(outputIndices, camPos);
		for (size_t i = 0; i < this->nodeIndices.size(); ++i)
			outputIndices.push_back(this->nodeIndices[i]);
		secondNode->traverseBackToFront(outputIndices, camPos);
	}*/

	firstNode->traverseBackToFront(outputIndices, camPos);
	for (size_t i = 0; i < this->nodeIndices.size(); ++i)
		outputIndices.push_back(this->nodeIndices[i]);
	secondNode->traverseBackToFront(outputIndices, camPos);
}

/*void BSPNode::traverseFrontToBack(std::vector<uint32_t>& outputIndices, const glm::vec3& camPos)
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
}*/

void BSPNode::assignSpaceIndices(std::vector<uint32_t>& indices)
{
	this->tempNodeIndices.assign(indices.begin(), indices.end());
}

void BSPNode::getTreeDepth(uint32_t& value)
{
	if (!this->negativeChild && !this->positiveChild)
	{
		value = std::max(value, this->depthLevel);
		return;
	}

	this->negativeChild->getTreeDepth(value);
	this->positiveChild->getTreeDepth(value);
}
