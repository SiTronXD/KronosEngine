#pragma once

#include <optional>

#include "../Dev/Log.h"
#include "../Application/Window.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "CommandBufferArray.h"
#include "QueueFamilies.h"

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

class Renderer
{
private:
	VkInstance instance;

	VkDebugUtilsMessengerEXT debugMessenger;

	VkSurfaceKHR surface;

	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	VkDevice device;

	QueueFamilies queueFamilies;

	VkSwapchainKHR swapChain;
	std::vector<VkImage> swapChainImages;
	std::vector<VkImageView> swapChainImageViews;
	std::vector<VkFramebuffer> swapChainFramebuffers;
	VkFormat swapChainImageFormat;
	VkExtent2D swapChainExtent;

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;
	VkPipelineLayout pipelineLayout;
	VkPipeline graphicsPipeline;

	CommandPool commandPool;
	CommandBufferArray commandBuffers;

	VkDescriptorPool descriptorPool;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	Texture depthTexture;
	Texture texture;

	uint32_t currentFrame = 0;

	Window* window;

	void initVulkan();

	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createDescriptorSetLayout();
	void createGraphicsPipeline();
	void createFramebuffers();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void createSyncObjects();

	void updateUniformBuffer(uint32_t currentImage);

	void cleanupSwapChain();
	void recreateSwapChain();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	VkSurfaceFormatKHR chooseSwapSurfaceFormat(
		const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(
		const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
	VkShaderModule createShaderModule(const std::vector<char>& code);
	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);
	bool hasStencilComponent(VkFormat format);

	void recordCommandBuffer(uint32_t imageIndex);

public:
	bool framebufferResized = false;

	Renderer();
	~Renderer();

	void init();
	void setWindow(Window& window);
	void cleanup();

	void drawFrame();

	// Vulkan
	inline CommandPool& getCommandPool() { return this->commandPool; }
	inline VkPhysicalDevice& getPhysicalDevice() { return this->physicalDevice; }
	inline VkDevice& getDevice() { return this->device; }
	inline QueueFamilies& getQueueFamilies() { return this->queueFamilies; }
};