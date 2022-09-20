#include "FramebufferArray.h"

#include "../Renderer.h"
#include "../Swapchain.h"
#include "../Texture.h"
#include "RenderPass.h"

FramebufferArray::FramebufferArray(Renderer& renderer)
	: renderer(renderer)
{
}

FramebufferArray::~FramebufferArray()
{
	this->cleanup();
}

void FramebufferArray::createFramebuffers(
	Texture& depthTexture,
	Swapchain& swapchain,
	RenderPass& renderPass)
{
	// Create one framebuffer for each swapchain image view
	this->framebuffers.resize(swapchain.getImageCount());
	for (size_t i = 0; i < swapchain.getImageCount(); ++i)
	{
		std::vector<VkImageView> attachments = { swapchain.getImageView(i) };
		if (renderPass.getBindDepth())
			attachments.push_back(depthTexture.getVkImageView());

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->renderer.getRenderPass().getVkRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapchain.getWidth();
		framebufferInfo.height = swapchain.getHeight();
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(
			this->renderer.getVkDevice(),
			&framebufferInfo,
			nullptr,
			&this->framebuffers[i]) != VK_SUCCESS)
		{
			Log::error("Failed to create framebuffer.");
		}
	}
}

void FramebufferArray::createImguiFramebuffers(
	Swapchain& swapchain, 
	RenderPass& imguiRenderPass)
{
	this->framebuffers.resize(swapchain.getImageCount());
	for (size_t i = 0; i < swapchain.getImageCount(); ++i)
	{
		std::array<VkImageView, 1> attachments =
		{
			swapchain.getImageView(i)
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = imguiRenderPass.getVkRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapchain.getWidth();
		framebufferInfo.height = swapchain.getHeight();
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(
			this->renderer.getVkDevice(),
			&framebufferInfo,
			nullptr,
			&this->framebuffers[i]) != VK_SUCCESS)
		{
			Log::error("Failed to create imgui framebuffer.");
		}
	}
}

void FramebufferArray::cleanup()
{
	for (auto framebuffer : this->framebuffers)
		vkDestroyFramebuffer(this->renderer.getVkDevice(), framebuffer, nullptr);

	this->framebuffers.clear();
}
