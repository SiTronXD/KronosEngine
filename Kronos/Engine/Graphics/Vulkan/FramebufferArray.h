#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class Renderer;
class Swapchain;
class RenderPass;
class Texture;

class FramebufferArray
{
private:
	std::vector<VkFramebuffer> framebuffers;

	Renderer& renderer;

public:
	FramebufferArray(Renderer& renderer);
	~FramebufferArray();

	void createFramebuffers(
		Texture& depthTexture,
		Swapchain& swapchain,
		RenderPass& renderPass);
	void createImguiFramebuffers(
		Swapchain& swapchain, 
		RenderPass& imguiRenderPass);

	void cleanup();

	inline VkFramebuffer& getVkFramebuffer(const uint32_t& index)
		{ return this->framebuffers[index]; }
};