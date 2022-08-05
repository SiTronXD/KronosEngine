#include "Texture.h"

#include "../Dev/Log.h"
#include "Renderer.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

bool Texture::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		format == VK_FORMAT_D24_UNORM_S8_UINT;
}

uint32_t Texture::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(this->renderer.getPhysicalDevice(), &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
	{
		if (typeFilter & (1 << i) &&
			(memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			return i;
		}
	}

	Log::error("Failed to find suitable memory type.");
	return uint32_t(~0);
}

void Texture::createBuffer(
	VkDeviceSize size, 
	VkBufferUsageFlags usage, 
	VkMemoryPropertyFlags properties, 
	VkBuffer& buffer, 
	VkDeviceMemory& bufferMemory)
{
	// Create buffer
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	if (vkCreateBuffer(
		this->renderer.getDevice(),
		&bufferInfo,
		nullptr,
		&buffer) != VK_SUCCESS)
	{
		Log::error("Failed to create buffer.");
	}

	// Get memory requirements from buffer
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(
		this->renderer.getDevice(),
		buffer,
		&memRequirements
	);

	// Allocate memory for buffer
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(
		memRequirements.memoryTypeBits,
		properties
	);
	if (vkAllocateMemory(
		this->renderer.getDevice(),
		&allocInfo,
		nullptr,
		&bufferMemory) != VK_SUCCESS)
	{
		Log::error("Failed to allocate vertex buffer memory.");
	}

	// Bind memory to buffer
	vkBindBufferMemory(
		this->renderer.getDevice(),
		buffer,
		bufferMemory,
		0
	);
}

void Texture::createImage(
	uint32_t width, 
	uint32_t height, 
	VkFormat format, 
	VkImageTiling tiling, 
	VkImageUsageFlags usage, 
	VkMemoryPropertyFlags properties, 
	VkImage& image, 
	VkDeviceMemory& imageMemory)
{
	// Create image
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.extent.width = width;
	imageInfo.extent.height = height;
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.format = format;
	imageInfo.tiling = tiling;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage = usage;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Used only by the graphics queue
	if (vkCreateImage(this->renderer.getDevice(), &imageInfo, nullptr, &image) != VK_SUCCESS)
	{
		Log::error("Failed to create image.");
	}

	// Get memory requirements for image
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(this->renderer.getDevice(), image, &memRequirements);

	// Allocate memory for image
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(
		memRequirements.memoryTypeBits,
		properties
	);
	if (vkAllocateMemory(this->renderer.getDevice(), &allocInfo, nullptr, &imageMemory) != VK_SUCCESS)
	{
		Log::error("Failed to allocate image memory.");
	}

	// Bind memory to image
	vkBindImageMemory(
		this->renderer.getDevice(),
		image,
		imageMemory,
		0
	);
}

VkImageView Texture::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
{
	// Image view info
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	viewInfo.subresourceRange.aspectMask = aspectFlags;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	// Create image view
	VkImageView imageView;
	if (vkCreateImageView(this->renderer.getDevice(), &viewInfo, nullptr, &imageView))
		Log::error("Failed to create texture image view.");

	return imageView;
}

void Texture::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{
	VkCommandBuffer commandBuffer = this->renderer.beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // To transfer queue family ownership
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED; // To transfer queue family ownership
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = 1;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = 1;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = 0;

	// Update aspect mask for depth/stencil image
	if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (this->hasStencilComponent(format))
			barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	// Access mask: "how is it accessed?"
	// Src/Dst stage: "when can the transfer take place?"

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
		newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
		newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
	{
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT |
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	}
	else
	{
		Log::error("Unsupported layout transition.");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage, destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	this->renderer.endSingleTimeCommands(commandBuffer);
}

void Texture::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
	VkCommandBuffer commandBuffer = this->renderer.beginSingleTimeCommands();

	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;

	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = 1;

	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };

	vkCmdCopyBufferToImage(
		commandBuffer,
		buffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&region
	);

	this->renderer.endSingleTimeCommands(commandBuffer);
}

bool Texture::createTextureImage(const std::string& filePath)
{
	// Load image
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load(
		filePath.c_str(),
		&texWidth,
		&texHeight,
		&texChannels,
		STBI_rgb_alpha
	);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		Log::error("Failed to load texture image.");
		return false;
	}

	// Staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	this->createBuffer(
		imageSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	// Copy data to staging buffer
	void* data;
	vkMapMemory(this->renderer.getDevice(), stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(this->renderer.getDevice(), stagingBufferMemory);

	// Free pixel array
	stbi_image_free(pixels);

	// Create image
	this->createImage(
		texWidth,
		texHeight,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		this->textureImage,
		this->textureImageMemory
	);

	// TODO: transition and copy could be setup inside a single
	// command buffer to increase throughput

	// Transition layout to transfer dst
	this->transitionImageLayout(
		this->textureImage,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
	);

	// Copy from buffer to image
	this->copyBufferToImage(
		stagingBuffer,
		this->textureImage,
		static_cast<uint32_t>(texWidth),
		static_cast<uint32_t>(texHeight)
	);

	// Transition layout to shader read
	this->transitionImageLayout(
		this->textureImage,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
	);

	// Deallocate staging buffer
	vkDestroyBuffer(this->renderer.getDevice(), stagingBuffer, nullptr);
	vkFreeMemory(this->renderer.getDevice(), stagingBufferMemory, nullptr);

	return true;
}

bool Texture::createTextureImageView()
{
	this->textureImageView = this->createImageView(
		this->textureImage,
		VK_FORMAT_R8G8B8A8_SRGB,
		VK_IMAGE_ASPECT_COLOR_BIT
	);

	return true;
}

bool Texture::createTextureSampler()
{
	// Get physical device properties
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(this->renderer.getPhysicalDevice(), &properties);

	// Sampler create info
	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter = VK_FILTER_LINEAR;
	samplerInfo.minFilter = VK_FILTER_LINEAR;
	samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable = VK_TRUE;
	samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.compareEnable = VK_FALSE;
	samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	samplerInfo.mipLodBias = 0.0f;
	samplerInfo.minLod = 0.0f;
	samplerInfo.maxLod = 0.0f;

	// Create sampler
	if (vkCreateSampler(
		this->renderer.getDevice(), 
		&samplerInfo, 
		nullptr, 
		&this->textureSampler) != VK_SUCCESS)
	{
		Log::error("Failed to create texture sampler.");
		return false;
	}

	return true;
}

Texture::Texture(Renderer& renderer)
	: textureImage(VK_NULL_HANDLE),
	textureImageMemory(VK_NULL_HANDLE),
	textureImageView(VK_NULL_HANDLE),
	textureSampler(VK_NULL_HANDLE),
	renderer(renderer)
{
}

Texture::~Texture()
{
}

bool Texture::createFromFile(const std::string& filePath)
{
	bool hasCreatedTextureImage = this->createTextureImage(filePath);
	bool hasCreatedTextureImageView = this->createTextureImageView();
	bool hasCreatedTextureSampler = this->createTextureSampler();

	return hasCreatedTextureImage && 
		hasCreatedTextureImageView && 
		hasCreatedTextureSampler;
}

void Texture::cleanup()
{
	vkDestroySampler(this->renderer.getDevice(), this->textureSampler, nullptr);
	vkDestroyImageView(this->renderer.getDevice(), this->textureImageView, nullptr);
	vkDestroyImage(this->renderer.getDevice(), this->textureImage, nullptr);
	vkFreeMemory(this->renderer.getDevice(), this->textureImageMemory, nullptr);
}
