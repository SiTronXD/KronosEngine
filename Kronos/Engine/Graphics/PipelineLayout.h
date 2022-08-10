#pragma once

#include <vulkan/vulkan.h>

class Renderer;

class PipelineLayout
{
private:
	VkPipelineLayout pipelineLayout;

	Renderer& renderer;

public:
	PipelineLayout(Renderer& renderer);
	~PipelineLayout();

	void createPipelineLayout(const VkDescriptorSetLayout& descriptorSetLayout);

	void cleanup();

	inline const VkPipelineLayout& getPipelineLayout() const { return this->pipelineLayout; }
};