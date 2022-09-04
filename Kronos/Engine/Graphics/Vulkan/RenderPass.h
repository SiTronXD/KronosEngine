#pragma once

#include <vulkan/vulkan.h>

class Renderer;

class RenderPass
{
private:
	VkRenderPass renderPass;

	Renderer& renderer;

public:
	RenderPass(Renderer& renderer);
	~RenderPass();

	void createRenderPass();
	void createImguiRenderPass();

	void cleanup();

	inline const VkRenderPass& getVkRenderPass() const { return this->renderPass; }
};