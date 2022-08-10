#include "PipelineLayout.h"

#include "Renderer.h"

PipelineLayout::PipelineLayout(Renderer& renderer)
	: renderer(renderer),
	pipelineLayout(VK_NULL_HANDLE)
{
}

PipelineLayout::~PipelineLayout()
{
}

void PipelineLayout::createPipelineLayout(const VkDescriptorSetLayout& descriptorSetLayout)
{
	// Pipeline layout (uniforms and push values)
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	if (vkCreatePipelineLayout(
		this->renderer.getDevice(),
		&pipelineLayoutInfo,
		nullptr,
		&this->pipelineLayout) != VK_SUCCESS)
	{
		Log::error("Failed to create pipeline layout!");
	}
}

void PipelineLayout::cleanup()
{
	vkDestroyPipelineLayout(this->renderer.getDevice(), this->pipelineLayout, nullptr);
}