#pragma once

#include "../Application/Window.h"

class Renderer;

class Buffer
{
public:
	Buffer();
	virtual ~Buffer();

	static uint32_t findMemoryType(
		VkPhysicalDevice physicalDevice, 
		uint32_t typeFilter,
		VkMemoryPropertyFlags properties);
	static void createBuffer(
		VkPhysicalDevice physicalDevice,
		VkDevice device,
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);
	static void copyBuffer(
		Renderer& renderer,
		VkBuffer srcBuffer,
		VkBuffer dstBuffer,
		VkDeviceSize size);
};