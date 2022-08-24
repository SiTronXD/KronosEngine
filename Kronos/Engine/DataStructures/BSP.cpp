#include "BSP.h"
#include "../Dev/Log.h"

void BSP::deleteRoot()
{
	delete this->rootNode;
	this->rootNode = nullptr;
}

BSP::BSP()
	: rootNode(nullptr)
{
}

BSP::~BSP()
{
	this->deleteRoot();
}

void BSP::createFromMeshData(MeshData& meshData)
{
	/*Plane plane(
		glm::vec3(0.35f, 0.0f, 0.35f),
		glm::vec3(1.0f, 0.0f, 1.0f)
	);*/
	/*Plane plane(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 1.0f)
	);*/
	/*Plane plane(
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.1f, 1.0f)
	);*/

	// Create root node
	this->deleteRoot();
	this->rootNode = new BSPNode(0);

	// Make a copy of the mesh data
	std::vector<Vertex> vertices = meshData.getVertices();
	std::vector<uint32_t> indices = meshData.getIndices();

	// Split recursively
	this->rootNode->splitMesh(vertices, indices);

	// Get new indices
	std::vector<uint32_t> newIndices;
	newIndices.reserve(indices.size());
	this->rootNode->getMergedIndices(newIndices);

	// Write
	Log::write("Num verts: " + std::to_string(vertices.size()));
	Log::write("Num ind: " + std::to_string(newIndices.size()));

	// Apply 
	meshData.getVertices().assign(vertices.begin(), vertices.end());
	meshData.getIndices().assign(newIndices.begin(), newIndices.end());
}
