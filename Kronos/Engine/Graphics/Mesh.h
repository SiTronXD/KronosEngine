#pragma once

#include "VertexBuffer.h"
#include "IndexBuffer.h"

class Renderer;

class Mesh
{
private:
	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;

	size_t numIndices;

public:
	Mesh(Renderer& renderer);
	~Mesh();

	void createMesh(
		std::vector<Vertex>& vertices, 
		std::vector<uint32_t>& indices,
		bool cpuWriteToIndexBuffer = false);

	void cleanup();

	inline VertexBuffer& getVertexBuffer() { return this->vertexBuffer; }
	inline IndexBuffer& getIndexBuffer() { return this->indexBuffer; }

	inline const size_t& getNumIndices() const { return this->numIndices; }
};