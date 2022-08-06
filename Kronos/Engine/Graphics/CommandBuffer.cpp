#include "CommandBuffer.h"

#include "Renderer.h"

CommandBuffer::CommandBuffer()
	: commandBuffer(VK_NULL_HANDLE)
{
}

CommandBuffer::~CommandBuffer()
{
}

void CommandBuffer::resetAndBegin()
{
	// Reset command buffer
	vkResetCommandBuffer(this->commandBuffer, 0);

	// Reset and begin recording into command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;
	if (vkBeginCommandBuffer(this->commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		Log::error("Failed to begin recording command buffer.");
	}
}

void CommandBuffer::beginRenderPass(const VkRenderPassBeginInfo& renderPassInfo)
{
	// Record beginning render pass
	vkCmdBeginRenderPass(
		this->commandBuffer,
		&renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE
	);
}

void CommandBuffer::bindPipeline(const VkPipeline& pipeline)
{
	vkCmdBindPipeline(
		this->commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline
	);
}

void CommandBuffer::setViewport(const VkViewport& viewport)
{
	vkCmdSetViewport(this->commandBuffer, 0, 1, &viewport);
}

void CommandBuffer::setScissor(const VkRect2D& scissor)
{
	vkCmdSetScissor(this->commandBuffer, 0, 1, &scissor);
}

void CommandBuffer::bindVertexBuffer(VertexBuffer& vertexBuffer)
{
	VkBuffer vertexBuffers[] = { vertexBuffer.getBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(this->commandBuffer, 0, 1, vertexBuffers, offsets);
}

void CommandBuffer::bindIndexBuffer(IndexBuffer& indexBuffer)
{
	vkCmdBindIndexBuffer(this->commandBuffer, indexBuffer.getBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void CommandBuffer::bindDescriptorSet(
	const VkPipelineLayout& pipelineLayout, 
	const VkDescriptorSet& descriptorSet)
{
	vkCmdBindDescriptorSets(
		this->commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipelineLayout,
		0,
		1,
		&descriptorSet,
		0,
		nullptr
	);
}

void CommandBuffer::drawIndexed(size_t numIndices)
{
	// Record draw!
	vkCmdDrawIndexed(this->commandBuffer, static_cast<uint32_t>(numIndices), 1, 0, 0, 0);
}

void CommandBuffer::endPassAndRecording()
{
	// Record ending render pass
	vkCmdEndRenderPass(this->commandBuffer);

	// Finish recording
	if (vkEndCommandBuffer(this->commandBuffer) != VK_SUCCESS)
	{
		Log::error("Failed to record command buffer.");
	}
}

void CommandBuffer::setCommandBuffer(const VkCommandBuffer& commandBuffer)
{
	this->commandBuffer = commandBuffer;
}

VkCommandBuffer CommandBuffer::beginSingleTimeCommands(Renderer& renderer)
{
	VkCommandBuffer commandBuffer;

	// Allocate command buffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = renderer.getCommandPool().getCommandPool();
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(renderer.getDevice(), &allocInfo, &commandBuffer);

	// Begin recording command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void CommandBuffer::endSingleTimeCommands(
	Renderer& renderer,
	VkCommandBuffer commandBuffer)
{
	VkQueue& queue = renderer.getGraphicsQueue();

	// End recording command buffer
	vkEndCommandBuffer(commandBuffer);

	// Submit command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(queue);

	// Free command buffer
	vkFreeCommandBuffers(renderer.getDevice(), renderer.getCommandPool().getCommandPool(), 1, &commandBuffer);
}
