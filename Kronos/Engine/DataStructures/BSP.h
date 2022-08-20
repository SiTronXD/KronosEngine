#pragma once

#include "../Graphics/MeshData.h"

class BSP
{
private:
public:
	BSP();
	~BSP();

	void createFromMeshData(MeshData& meshData);
};