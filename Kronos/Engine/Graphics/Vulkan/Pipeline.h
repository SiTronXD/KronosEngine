#pragma once

#include "PipelineLayout.h"
#include "RenderPass.h"

class Pipeline
{
private:
	VkPipeline pipeline;
	VkPipelineCache pipelineCache;

	Renderer& renderer;

public:
	Pipeline(Renderer& renderer);
	~Pipeline();

	void createPipelineCache();
	void createGraphicsPipeline(
		PipelineLayout& pipelineLayout,
		const RenderPass& renderPass,
		bool wireframe = false,
		bool useDepthTesting = false,
		bool useStencilTesting = false);

	void cleanup();
	void cleanupPipelineCache();

	inline const VkPipeline& getVkPipeline() const { return this->pipeline; }
};