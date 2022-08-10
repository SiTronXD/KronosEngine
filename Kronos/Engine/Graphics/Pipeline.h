#pragma once

#include "PipelineLayout.h"

class Pipeline
{
private:
	VkPipeline pipeline;

	Renderer& renderer;

public:
	Pipeline(Renderer& renderer);
	~Pipeline();

	void createGraphicsPipeline(
		PipelineLayout& pipelineLayout,
		const VkRenderPass& renderPass);

	void cleanup();

	inline const VkPipeline& getPipeline() const { return this->pipeline; }
};