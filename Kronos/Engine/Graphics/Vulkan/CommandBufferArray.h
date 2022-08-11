#pragma once

#include "CommandPool.h"
#include "CommandBuffer.h"

class Renderer;

class CommandBufferArray
{
private:
	std::vector<CommandBuffer*> commandBuffers;
	std::vector<VkCommandBuffer> commandBufferData;

	Renderer& renderer;
	CommandPool& commandPool;

public:
	CommandBufferArray(Renderer& renderer, CommandPool& commandPool);
	~CommandBufferArray();

	void createCommandBuffers(size_t numCommandBuffers);

	void cleanup();

	inline CommandBuffer& getVkCommandBuffer(uint32_t commandBufferIndex) 
		{ return *this->commandBuffers[commandBufferIndex]; }
};