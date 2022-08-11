#pragma once

#include <vulkan/vulkan.h>

class Renderer;

class CommandPool
{
private:
	VkCommandPool commandPool;

	Renderer& renderer;

public:
	CommandPool(Renderer& renderer);
	~CommandPool();

	void create();
	void cleanup();

	inline VkCommandPool& getVkCommandPool() { return this->commandPool; }
};