#pragma once

#include <vector>
#include <vulkan/vulkan.h>

class Window;

class Instance
{
private:
	VkInstance instance;

public:
	Instance();
	~Instance();

	void createInstance(
		bool enableValidationLayers, 
		const std::vector<const char*>& validationLayers,
		Window* window);

	void cleanup();

	inline VkInstance& getVkInstance() { return this->instance; }
};

