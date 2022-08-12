#include "DescriptorPool.h"

#include "../Renderer.h"

DescriptorPool::DescriptorPool(Renderer& renderer)
	: renderer(renderer),
	descriptorPool(VK_NULL_HANDLE)
{
}

DescriptorPool::~DescriptorPool() {}

void DescriptorPool::createDescriptorPool(uint32_t descriptorCount)
{
	// Pool size
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = descriptorCount;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = descriptorCount;

	// Create descriptor pool
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = descriptorCount;
	poolInfo.flags = 0;
	if (vkCreateDescriptorPool(
		this->renderer.getVkDevice(),
		&poolInfo,
		nullptr,
		&this->descriptorPool) != VK_SUCCESS)
	{
		Log::error("Failed to create descriptor pool.");
	}
}

void DescriptorPool::cleanup()
{
	// Destroys descriptor sets allocated from it
	vkDestroyDescriptorPool(this->renderer.getVkDevice(), this->descriptorPool, nullptr);
}
