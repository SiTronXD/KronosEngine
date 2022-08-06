#include "VertexBuffer.h"

#include "Renderer.h"

VertexBuffer::VertexBuffer(Renderer& renderer)
	: Buffer(renderer)
{
}

VertexBuffer::~VertexBuffer()
{
}

void VertexBuffer::createVertexBuffer(const std::vector<Vertex>& vertices)
{
	// Resusable references
	Renderer& renderer = Buffer::getRenderer();
	VkPhysicalDevice& physicalDevice =
		renderer.getPhysicalDevice();
	VkDevice& device =
		renderer.getDevice();

	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

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

	// Fill buffer memory with data

	// Map buffer memory into CPU accessible memory
	void* data;
	vkMapMemory(
		device,
		stagingBufferMemory,
		0,
		bufferSize,
		0,
		&data
	);

	// Copy data to memory
	memcpy(data, vertices.data(), (size_t)bufferSize);

	// Unmap buffer memory
	vkUnmapMemory(device, stagingBufferMemory);

	// Create vertex buffer
	Buffer::createBuffer(
		physicalDevice,
		device,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
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