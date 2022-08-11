#include "CommandPool.h"

#include "../../Dev/Log.h"
#include "../Renderer.h"

CommandPool::CommandPool(Renderer& renderer)
	: commandPool(VK_NULL_HANDLE),
	renderer(renderer)
{
}

CommandPool::~CommandPool()
{
}

void CommandPool::create()
{
	QueueFamilyIndices& queueFamilyIndices =
		this->renderer.getQueueFamilies().getIndices();

	// Create command pool for graphics queue
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	if (vkCreateCommandPool(
		this->renderer.getVkDevice(),
		&poolInfo,
		nullptr,
		&this->commandPool) != VK_SUCCESS)
	{
		Log::error("Failed to create command pool.");
	}
}

void CommandPool::cleanup()
{
	// Destroys command pool and command buffers allocated from it
	vkDestroyCommandPool(this->renderer.getVkDevice(), this->commandPool, nullptr);
}