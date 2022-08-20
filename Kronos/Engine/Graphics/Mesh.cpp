#include "Mesh.h"

#include "Renderer.h"

Mesh::Mesh(Renderer& renderer)
	: vertexBuffer(renderer), 
	indexBuffer(renderer),
	numIndices(0)
{
}

Mesh::~Mesh()
{
}

void Mesh::createMesh(
	std::vector<Vertex>& vertices,
	std::vector<uint32_t>& indices,
	bool cpuWriteToIndexBuffer)
{
	this->vertexBuffer.createVertexBuffer(vertices);
	this->indexBuffer.createIndexBuffer(indices, cpuWriteToIndexBuffer);

	this->numIndices = indices.size();
}

void Mesh::cleanup()
{
	this->indexBuffer.cleanup();
	this->vertexBuffer.cleanup();
}
