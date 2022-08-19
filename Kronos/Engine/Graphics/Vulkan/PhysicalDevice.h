#pragma once

#include <vector>

#include "QueueFamilies.h"

class Renderer;

class PhysicalDevice
{
private:
	VkPhysicalDevice physicalDevice;

	Renderer& renderer;

public:
	PhysicalDevice(Renderer& renderer);
	~PhysicalDevice();

	void pickPhysicalDevice(
		VkInstance instance, 
		VkSurfaceKHR surface,
		const std::vector<const char*>& deviceExtensions,
		QueueFamilies& outputQueueFamilies);

	void cleanup();

	inline VkPhysicalDevice& getVkPhysicalDevice() { return this->physicalDevice; }
};

