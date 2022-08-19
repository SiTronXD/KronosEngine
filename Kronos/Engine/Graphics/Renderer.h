#pragma once

#include <optional>

#include "../Dev/Log.h"
#include "../Application/Window.h"
#include "Vulkan/CommandBufferArray.h"
#include "Vulkan/Pipeline.h"
#include "Vulkan/DescriptorSetLayout.h"
#include "Vulkan/RenderPass.h"
#include "Vulkan/DescriptorPool.h"
#include "Vulkan/DescriptorSetArray.h"
#include "Vulkan/PhysicalDevice.h"
#include "Vulkan/Device.h"
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

	PhysicalDevice physicalDevice;
	Device device;
	QueueFamilies queueFamilies;
	Swapchain swapchain;

	RenderPass renderPass;
	DescriptorSetLayout descriptorSetLayout;
	PipelineLayout graphicsPipelineLayout;
	Pipeline graphicsPipeline;

	CommandPool commandPool;
	CommandBufferArray commandBuffers;

	DescriptorPool descriptorPool;
	DescriptorSetArray descriptorSets;

	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;
	std::vector<VkFence> inFlightFences;

	VertexBuffer vertexBuffer;
	IndexBuffer indexBuffer;

	std::vector<VkBuffer> uniformBuffers;
	std::vector<VkDeviceMemory> uniformBuffersMemory;

	Texture texture;

	Window* window;

	uint32_t currentFrame = 0;

	void initVulkan();

	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void createUniformBuffers();
	void createSyncObjects();

	void updateUniformBuffer(uint32_t currentImage, Camera& camera);

	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

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
	inline VkPhysicalDevice& getVkPhysicalDevice() { return this->physicalDevice.getVkPhysicalDevice(); }
	inline VkDevice& getVkDevice() { return this->device.getVkDevice(); }
	inline VkSurfaceKHR& getVkSurface() { return this->surface; }

	inline CommandPool& getCommandPool() { return this->commandPool; }
	inline QueueFamilies& getQueueFamilies() { return this->queueFamilies; }
	inline RenderPass& getRenderPass() { return this->renderPass; }
	inline Swapchain& getSwapchain() { return this->swapchain; }
	inline Window& getWindow() { return *this->window; }

	inline float getSwapchainAspectRatio() 
		{ return (float) this->swapchain.getWidth() / this->swapchain.getHeight(); }
};