#pragma once

#include <optional>

#include "../Dev/Log.h"
#include "../Application/Window.h"
#include "Texture.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "CommandBufferArray.h"
#include "QueueFamilies.h"
#include "Swapchain.h"
#include "Camera.h"
#include "Pipeline.h"

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

	VkRenderPass renderPass;
	VkDescriptorSetLayout descriptorSetLayout;

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
	void createRenderPass();
	void createDescriptorSetLayout();
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

	inline VkPhysicalDevice& getPhysicalDevice() { return this->physicalDevice; }
	inline VkDevice& getDevice() { return this->device; }
	inline VkSurfaceKHR& getSurface() { return this->surface; }
	inline VkRenderPass& getRenderPass() { return this->renderPass; }
	inline Window& getWindow() { return *this->window; }

	inline float getSwapchainAspectRatio() { return (float) this->swapchain.getWidth() / this->swapchain.getHeight(); }
};