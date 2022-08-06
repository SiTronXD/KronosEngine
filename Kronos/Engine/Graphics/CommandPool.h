#pragma once

#include "../Application/Window.h"

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

	inline VkCommandPool& getCommandPool() { return this->commandPool; }
};