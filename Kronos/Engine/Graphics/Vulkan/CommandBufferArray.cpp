#include "CommandBufferArray.h"

#include "../Renderer.h"

CommandBufferArray::CommandBufferArray(Renderer& renderer, CommandPool& commandPool)
	: renderer(renderer), commandPool(commandPool)
{
}

CommandBufferArray::~CommandBufferArray()
{
	this->cleanup();
}

void CommandBufferArray::createCommandBuffers(size_t numCommandBuffers)
{
	this->commandBuffers.resize(numCommandBuffers);
	this->commandBufferData.resize(numCommandBuffers);

	// Allocate command buffer from command pool
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = this->commandPool.getVkCommandPool();
	allocInfo.commandBufferCount = (uint32_t) numCommandBuffers;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	if (vkAllocateCommandBuffers(
		this->renderer.getVkDevice(),
		&allocInfo,
		this->commandBufferData.data()) != VK_SUCCESS)
	{
		Log::error("Failed to allocate command buffers.");
	}

	// Set command buffer ptr in objects
	for (size_t i = 0; i < this->commandBuffers.size(); ++i)
	{
		this->commandBuffers[i] = new CommandBuffer();
		this->commandBuffers[i]->setCommandBuffer(this->commandBufferData[i]);
	}
}

void CommandBufferArray::cleanup()
{
	for (size_t i = 0; i < this->commandBuffers.size(); ++i)
	{
		delete this->commandBuffers[i];
		this->commandBuffers[i] = nullptr;
	}

	this->commandBuffers.clear();
}
