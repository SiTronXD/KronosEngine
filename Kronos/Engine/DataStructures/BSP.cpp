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

void BSP::traverseTree(MeshData& meshData, const glm::vec3& camPos)
{
	// New indices for this frame
	meshData.getIndices().clear();

	// Fill new indices by traversing tree
	this->rootNode->traverseBackToFront(meshData.getIndices(), camPos);
}
