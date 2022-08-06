#pragma once

#include <vector>

#include "CommandPool.h"

class VertexBuffer;
class IndexBuffer;

class CommandBuffer
{
private:
	VkCommandBuffer commandBuffer;

public:
	CommandBuffer();
	~CommandBuffer();

	void resetAndBegin();

	void beginRenderPass(const VkRenderPassBeginInfo& renderPassInfo);
	void bindPipeline(const VkPipeline& pipeline);
	void setViewport(const VkViewport& viewport);
	void setScissor(const VkRect2D& scissor);
	void bindVertexBuffer(VertexBuffer& vertexBuffer);
	void bindIndexBuffer(IndexBuffer& indexBuffer);
	void bindDescriptorSet(
		const VkPipelineLayout& pipelineLayout,
		const VkDescriptorSet& descriptorSet);
	void drawIndexed(size_t numIndices);

	void endPassAndRecording();

	void setCommandBuffer(const VkCommandBuffer& commandBuffer);

	inline VkCommandBuffer& getCommandBuffer() { return this->commandBuffer; }


	static VkCommandBuffer beginSingleTimeCommands(Renderer& renderer);
	static void endSingleTimeCommands(
		Renderer& renderer,
		VkCommandBuffer commandBuffer);
};