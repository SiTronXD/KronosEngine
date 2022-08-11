#pragma once

#include <optional>

#include "../Dev/Log.h"
#include "../Application/Window.h"
#include "Vulkan/CommandBufferArray.h"
#include "Vulkan/QueueFamilies.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/DescriptorSetLayout.h"
#include "Vulkan/RenderPass.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Swapchain.h"
#include "Camera.h"

struct UniformBufferObject
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
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
	Swapchain swapchain;

	RenderPass renderPass;
	DescriptorSetLayout descriptorSetLayout;
	PipelineLayout graphicsPipelineLayout;
	Pipeline graphicsPipeline;

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

	Texture texture;

	uint32_t currentFrame = 0;

	Window* window;

	void initVulkan();

	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
	void createUniformBuffers();
	void createDescriptorPool();
	void createDescriptorSets();
	void createSyncObjects();

	void updateUniformBuffer(uint32_t currentImage, Camera& camera);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

	std::vector<const char*> getRequiredExtensions();
	bool checkValidationLayerSupport();
	bool checkDeviceExtensionSupport(VkPhysicalDevice device);
	bool isDeviceSuitable(VkPhysicalDevice device);

	void recordCommandBuffer(uint32_t imageIndex);

public:
	bool framebufferResized = false;

	Renderer();
	~Renderer();

	void init();
	void setWindow(Window& window);
	void cleanup();

	void drawFrame(Camera& camera);

	// Vulkan
	inline CommandPool& getCommandPool() { return this->commandPool; }
	inline QueueFamilies& getQueueFamilies() { return this->queueFamilies; }
	inline RenderPass& getRenderPass() { return this->renderPass; }
	inline Swapchain& getSwapchain() { return this->swapchain; }
	inline Window& getWindow() { return *this->window; }

	inline VkPhysicalDevice& getVkPhysicalDevice() { return this->physicalDevice; }
	inline VkDevice& getVkDevice() { return this->device; }
	inline VkSurfaceKHR& getVkSurface() { return this->surface; }

	inline float getSwapchainAspectRatio() 
		{ return (float) this->swapchain.getWidth() / this->swapchain.getHeight(); }
};