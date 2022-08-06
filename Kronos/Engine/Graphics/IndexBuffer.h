#pragma once

#include "Buffer.h"

class IndexBuffer : public Buffer
{
private:
public:
	IndexBuffer(Renderer& renderer);
	~IndexBuffer();

	void createIndexBuffer(const std::vector<uint32_t>& indices);
};