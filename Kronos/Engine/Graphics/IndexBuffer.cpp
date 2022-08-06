#include "IndexBuffer.h"

#include "Renderer.h"

IndexBuffer::IndexBuffer(Renderer& renderer)
	: Buffer(renderer)
{
}

IndexBuffer::~IndexBuffer()
{
}

void IndexBuffer::createIndexBuffer(const std::vector<uint32_t>& indices)
{
	// Resusable references
	Renderer& renderer = Buffer::getRenderer();
	VkPhysicalDevice& physicalDevice =
		renderer.getPhysicalDevice();
	VkDevice& device =
		renderer.getDevice();

	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	// Create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	Buffer::createBuffer(
		physicalDevice,
		device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	// Map memory
	void* data;
	vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(device, stagingBufferMemory);

	// Create real buffer
	Buffer::createBuffer(
		physicalDevice,
		device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		Buffer::getBuffer(),
		Buffer::getBufferMemory()
	);

	// Copy from staging buffer to real buffer
	Buffer::copyBuffer(renderer, stagingBuffer, Buffer::getBuffer(), bufferSize);

	// Deallocate staging buffer
	vkDestroyBuffer(device, stagingBuffer, nullptr);
	vkFreeMemory(device, stagingBufferMemory, nullptr);
}
