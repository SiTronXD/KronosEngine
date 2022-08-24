#pragma once

#include "BSPNode.h"

class BSP
{
private:
	BSPNode* rootNode;

	void deleteRoot();

public:
	BSP();
	~BSP();

	void createFromMeshData(MeshData& meshData);
};