#pragma once

#include <string>

#include "../Application/Window.h"

class Renderer;

class Texture
{
private:
	VkImage textureImage;
	VkDeviceMemory textureImageMemory;
	VkImageView textureImageView;
	VkSampler textureSampler;

	Renderer& renderer;

	bool hasStencilComponent(VkFormat format);
	uint32_t findMemoryType(
		uint32_t typeFilter,
		VkMemoryPropertyFlags properties);
	void createBuffer(
		VkDeviceSize size,
		VkBufferUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkBuffer& buffer,
		VkDeviceMemory& bufferMemory);
	void createImage(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory);
	VkImageView createImageView(
		VkImage image,
		VkFormat format,
		VkImageAspectFlags aspectFlags);
	void transitionImageLayout(
		VkImage image,
		VkFormat format,
		VkImageLayout oldLayout,
		VkImageLayout newLayout);
	void copyBufferToImage(
		VkBuffer buffer, VkImage image,
		uint32_t width, uint32_t height);

	bool createTextureImage(const std::string& filePath);
	bool createTextureImageView();
	bool createTextureSampler();

public:
	Texture(Renderer& renderer);
	~Texture();

	bool createFromFile(const std::string& filePath);

	void cleanup();

	// Vulkan
	inline VkImage& getImage() { return this->textureImage; }
	inline VkDeviceMemory& getImageMemory() { return this->textureImageMemory; }
	inline VkImageView& getImageView() { return this->textureImageView; }
	inline VkSampler& getSampler() { return this->textureSampler; }
};