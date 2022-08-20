#include "BSP.h"

BSP::BSP()
{
}

BSP::~BSP()
{
}

void BSP::createFromMeshData(MeshData& meshData)
{
	for (size_t i = 0; i < meshData.getVertices().size(); ++i)
	{
		Vertex& v = meshData.getVertices()[i];

		if (v.pos.x < 0.0f)
		{
			meshData.getVertices()[i].color.r = 0.5f;
			meshData.getVertices()[i].color.g = 0.0f;
			meshData.getVertices()[i].color.b = 0.0f;
		}
		else
		{
			meshData.getVertices()[i].color.r = 0.0f;
			meshData.getVertices()[i].color.g = 0.5f;
			meshData.getVertices()[i].color.b = 0.0f;
		}
	}
}
