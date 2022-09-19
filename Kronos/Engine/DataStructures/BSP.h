#pragma once

#include "BSPNode.h"

enum class BspTraversalMode
{
	BACK_TO_FRONT,
	FRONT_TO_BACK
};

class BSP
{
private:
	BSPNode* rootNode;

	void (BSPNode::*traverseFunction)
		(std::vector<uint32_t>& outputIndices, const glm::vec3& camPos);

	void deleteRoot();

public:
	BSP();
	~BSP();

	void createFromMeshData(MeshData& meshData);
	void traverseTree(MeshData& meshData, const glm::vec3& camPos);

	void setTraversalMode(const BspTraversalMode& newMode);

	uint32_t getTreeDepth();
};