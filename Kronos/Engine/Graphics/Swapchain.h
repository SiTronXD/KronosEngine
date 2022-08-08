#pragma once

#include <vector>
#include <vulkan/vulkan.h>

#include "Texture.h"

struct SwapchainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Swapchain
{
private:
	VkSwapchainKHR swapchain;
	VkFormat imageFormat;
	VkExtent2D extent;
	std::vector<VkImage> images;
	std::vector<VkImageView> imageViews;
	std::vector<VkFramebuffer> framebuffers;

	Texture depthTexture;

	Renderer& renderer;

	void createImageViews();

	void chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats,
		VkSurfaceFormatKHR& output);
	void chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>& availablePresentModes,
		VkPresentModeKHR& output);
	void chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities,
		VkExtent2D& output);

public:
	Swapchain(Renderer& renderer);
	~Swapchain();

	void createSwapchain();
	void createFramebuffers();

	void recreate();
	void cleanup();

	static void querySwapChainSupport(
		VkSurfaceKHR surface, 
		VkPhysicalDevice device,
		SwapchainSupportDetails& output);

	inline const VkSwapchainKHR& getSwapchain() { return this->swapchain; }
	inline const VkFormat& getFormat() const { return this->imageFormat; }
	inline const VkExtent2D& getExtent() const { return this->extent; }
	inline const VkFramebuffer& getFramebuffer(const uint32_t& index) { return this->framebuffers[index]; }
	inline const uint32_t& getWidth() const { return this->extent.width; }
	inline const uint32_t& getHeight() const { return this->extent.height; }
};