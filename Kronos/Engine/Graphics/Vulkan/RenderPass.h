#pragma once

#include <vulkan/vulkan.h>

class Renderer;

class RenderPass
{
private:
	VkRenderPass renderPass;

	Renderer& renderer;

	bool bindDepth;

public:
	RenderPass(Renderer& renderer);
	~RenderPass();

	void createRenderPass(bool bindDepth = false);
	void createImguiRenderPass();

	void cleanup();

	inline const VkRenderPass& getVkRenderPass() const { return this->renderPass; }
	inline const bool& getBindDepth() const { return this->bindDepth; }
};