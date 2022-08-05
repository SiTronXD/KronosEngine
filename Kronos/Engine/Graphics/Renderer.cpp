#include "Renderer.h"

#include "../Dev/Log.h"

#include <iostream>
#include <string>
#include <fstream>
#include <set>
#include <chrono>
#include <algorithm>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;
const int MAX_FRAMES_IN_FLIGHT = 2;

const std::vector<Vertex> vertices =
{
	{{ -0.5f, -0.5f, 0.0f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f}},
	{{  0.5f, -0.5f, 0.0f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f}},
	{{  0.5f,  0.5f, 0.0f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f}},
	{{ -0.5f,  0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f}},

	{{ -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f}},
	{{  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f}},
	{{  0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f}},
	{{ -0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f, 1.0f }, { 1.0f, 1.0f}}
};

const std::vector<uint16_t> indices =
{
	0, 1, 2,
	2, 3, 0,

	4, 5, 6,
	6, 7, 4,
};

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static std::vector<char> readFile(const std::string& filename)
{
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open())
	{
		Log::error("Failed to open file.");
	}

	// Allocate buffer from read position at the end of the file
	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);

	// Read all of the file from the beginning
	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();
	return buffer;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

	if (func != nullptr)
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	else
		return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator)
{
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		func(instance, debugMessenger, pAllocator);
	}
}

void Renderer::initVulkan()
{
	this->createInstance();
	this->setupDebugMessenger();
	this->createSurface();
	this->pickPhysicalDevice();
	this->createLogicalDevice();
	this->createSwapChain();
	this->createImageViews();
	this->createRenderPass();
	this->createDescriptorSetLayout();
	this->createGraphicsPipeline();
	this->createCommandPool();
	
	this->depthTexture.createAsDepthTexture(
		this->swapChainExtent.width,
		this->swapChainExtent.height
	);

	this->createFramebuffers();

	this->texture.createFromFile("Resources/Textures/poggers.PNG");

	this->createVertexBuffer();
	this->createIndexBuffer();
	this->createUniformBuffers();
	this->createDescriptorPool();
	this->createDescriptorSets();
	this->createCommandBuffers();
	this->createSyncObjects();
}

void Renderer::cleanup()
{
	// Wait for device before cleanup
	vkDeviceWaitIdle(this->device);

	this->cleanupSwapChain();

	this->texture.cleanup();

	// Destroys descriptor sets allocated from it
	vkDestroyDescriptorPool(this->device, this->descriptorPool, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroyBuffer(this->device, this->uniformBuffers[i], nullptr);
		vkFreeMemory(this->device, this->uniformBuffersMemory[i], nullptr);
	}

	vkDestroyBuffer(this->device, this->indexBuffer, nullptr);
	vkFreeMemory(this->device, this->indexBufferMemory, nullptr);

	vkDestroyBuffer(this->device, this->vertexBuffer, nullptr);
	vkFreeMemory(this->device, this->vertexBufferMemory, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(this->device, this->imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(this->device, this->renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(this->device, this->inFlightFences[i], nullptr);
	}

	// Destroys command pool and command buffers allocated from it
	vkDestroyCommandPool(this->device, this->commandPool, nullptr);
	vkDestroyPipeline(this->device, this->graphicsPipeline, nullptr);
	vkDestroyDescriptorSetLayout(this->device, this->descriptorSetLayout, nullptr);
	vkDestroyPipelineLayout(this->device, this->pipelineLayout, nullptr);
	vkDestroyRenderPass(this->device, this->renderPass, nullptr);
	vkDestroyDevice(this->device, nullptr);

	if (enableValidationLayers)
		DestroyDebugUtilsMessengerEXT(this->instance, debugMessenger, nullptr);

	vkDestroySurfaceKHR(this->instance, this->surface, nullptr);

	// Destroys both physical device and instance
	vkDestroyInstance(this->instance, nullptr);
}

void Renderer::createInstance()
{
	if (enableValidationLayers && !checkValidationLayerSupport())
	{
		Log::error("Validation layers requested are not available.");
	}

	// Application info
	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Hello Triangle";
	appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_API_VERSION_1_0;

	// Instance create info
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Get and set extensions
	auto extensions = getRequiredExtensions();
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	// Validation layer debug info for specifically instance create/destroy
	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};

	// Validation layers
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		// Validation layer debug info for specifically instance create/destroy
		populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	// Create instance
	if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS)
	{
		Log::error("Failed to create instance.");
	}
}

void Renderer::setupDebugMessenger()
{
	if (!enableValidationLayers) return;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	populateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
	{
		Log::error("Failed to setup debug messenger.");
	}
}

void Renderer::createSurface()
{
	if (glfwCreateWindowSurface(
		this->instance,
		this->window->getWindowHandle(),
		nullptr,
		&this->surface) != VK_SUCCESS)
	{
		Log::error("Failed to create window surface.");
		return;
	}
}

void Renderer::pickPhysicalDevice()
{
	// Get device count
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

	// No devices found
	if (deviceCount == 0)
	{
		Log::error("Failed to find GPUs with Vulkan support.");
		return;
	}

	// Get device handles
	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

	// Pick the first best found device
	for (const auto& device : devices)
	{
		if (isDeviceSuitable(device))
		{
			physicalDevice = device;
			break;
		}
	}

	if (physicalDevice == VK_NULL_HANDLE)
	{
		Log::error("Failed to find a suitable GPU.");
		return;
	}
}

void Renderer::createLogicalDevice()
{
	// ---------- Queue families to be used ----------
	QueueFamilyIndices indices = findQueueFamilies(this->physicalDevice);

	// Unique queue families to be used
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies =
	{
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	// Create queue create info structs
	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;

		queueCreateInfos.push_back(queueCreateInfo);
	}

	// ---------- Device features ----------

	// Device features
	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	// ---------- Logical device ----------

	// Logical device create info
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();

	createInfo.pEnabledFeatures = &deviceFeatures;

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();

	// Not used in newer versions of vulkan
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
	}
	else
	{
		createInfo.enabledLayerCount = 0;
	}

	// Create the logical device
	if (vkCreateDevice(
		this->physicalDevice,
		&createInfo,
		nullptr,
		&device) != VK_SUCCESS)
	{
		Log::error("Failed to create logical device!");
		return;
	}

	// Get graphics queue handle
	vkGetDeviceQueue(this->device, indices.graphicsFamily.value(), 0, &this->graphicsQueue);

	// Get present queue handle
	vkGetDeviceQueue(this->device, indices.presentFamily.value(), 0, &this->presentQueue);
}

void Renderer::createSwapChain()
{
	// Swap chain support
	SwapChainSupportDetails swapChainSupport =
		querySwapChainSupport(this->physicalDevice);

	// Format, present mode and extent
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	// Image count
	uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
	if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
	{
		imageCount = swapChainSupport.capabilities.maxImageCount;
	}

	// Swap chain create info
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = this->surface;

	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	// How swap chain images are used across multiple queue families
	QueueFamilyIndices indices = findQueueFamilies(this->physicalDevice);
	uint32_t queueFamilyIndices[] =
	{
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};
	if (indices.graphicsFamily != indices.presentFamily)
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0; // Optional
		createInfo.pQueueFamilyIndices = nullptr; // Optional
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;	// Clip pixels overlapped by other windows
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	// Create swapchain
	if (vkCreateSwapchainKHR(this->device, &createInfo, nullptr, &this->swapChain) != VK_SUCCESS)
	{
		Log::error("Failed to created swapchain.");
		return;
	}

	// We've only specified the minimum number of images, so the implementation
	// could create more.
	vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, nullptr);
	swapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(this->device, this->swapChain, &imageCount, swapChainImages.data());

	// Save format and extent
	this->swapChainImageFormat = surfaceFormat.format;
	this->swapChainExtent = extent;
}

void Renderer::createImageViews()
{
	// Create an image view for each swapchain image
	swapChainImageViews.resize(swapChainImages.size());
	for (size_t i = 0; i < swapChainImages.size(); ++i)
	{
		this->swapChainImageViews[i] = this->createImageView(
			this->swapChainImages[i],
			this->swapChainImageFormat,
			VK_IMAGE_ASPECT_COLOR_BIT
		);
	}
}

void Renderer::createRenderPass()
{
	// Color attachment
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Layout DURING subpass

	// Depth attachment
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = Texture::findDepthFormat(this->physicalDevice);
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // Don't store after drawing is done
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Layout DURING subpass

	// Subpass
	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	// Subpass dependency
	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
		VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
		VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	// Render pass
	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;
	if (vkCreateRenderPass(
		this->device,
		&renderPassInfo,
		nullptr,
		&this->renderPass) != VK_SUCCESS)
	{
		Log::error("Failed to create render pass.");
	}
}

void Renderer::createDescriptorSetLayout()
{
	// Uniform buffer object layout binding
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding = 0;
	uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount = 1;
	uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers = nullptr;

	// Combined image sampler layout binding
	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding = 1;
	samplerLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.descriptorCount = 1;
	samplerLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	samplerLayoutBinding.pImmutableSamplers = nullptr;

	// Descriptor set layout info, which contains all layout bindings
	std::array<VkDescriptorSetLayoutBinding, 2> bindings =
	{
		uboLayoutBinding,
		samplerLayoutBinding
	};
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	// Create descriptor set layout
	if (vkCreateDescriptorSetLayout(
		this->device,
		&layoutInfo,
		nullptr,
		&this->descriptorSetLayout))
	{
		Log::error("Failed to create descriptor set layout.");
	}
}

void Renderer::createGraphicsPipeline()
{
	auto vertShaderCode = readFile("Resources/Shaders/vert.spv");
	auto fragShaderCode = readFile("Resources/Shaders/frag.spv");

	// Destroy after creating the pipeline
	VkShaderModule vertShaderModule = this->createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = this->createShaderModule(fragShaderCode);

	// Vertex shader stage create info
	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";
	vertShaderStageInfo.pSpecializationInfo = nullptr; // For shader constants

	// Fragment shader stage create info
	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";
	fragShaderStageInfo.pSpecializationInfo = nullptr; // For shader constants

	VkPipelineShaderStageCreateInfo shaderStages[] =
	{
		vertShaderStageInfo,
		fragShaderStageInfo
	};

	// Vertex input
	auto bindingDescription = Vertex::getBindingDescription();
	auto attributeDescriptions = Vertex::getAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = 1;
	vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

	// Input assembly
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	// Viewport
	/*VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float) swapChainExtent.width;
	viewport.height = (float) swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Scissor
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChainExtent;*/

	// Dynamic states (for dynamic viewport)
	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates = dynamicStates.data();

	// Viewport state 
	// (Actual viewport/scissor is set at drawing time)
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.scissorCount = 1;

	// Rasterizer
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f;
	rasterizer.depthBiasClamp = 0.0f;
	rasterizer.depthBiasSlopeFactor = 0.0f;

	// Multisampling
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f;
	multisampling.pSampleMask = nullptr;
	multisampling.alphaToCoverageEnable = VK_FALSE;
	multisampling.alphaToOneEnable = VK_FALSE;

	// Color blend per attached framebuffer
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask =
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	// Global color blend
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;

	// Pipeline layout (uniforms and push values)
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = &this->descriptorSetLayout;
	pipelineLayoutInfo.pushConstantRangeCount = 0;
	pipelineLayoutInfo.pPushConstantRanges = nullptr;
	if (vkCreatePipelineLayout(
		this->device,
		&pipelineLayoutInfo,
		nullptr,
		&this->pipelineLayout) != VK_SUCCESS)
	{
		Log::error("Failed to create pipeline layout!");
	}

	// Depth/stencil state
	VkPipelineDepthStencilStateCreateInfo depthStencilState{};
	depthStencilState.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencilState.depthTestEnable = VK_TRUE;
	depthStencilState.depthWriteEnable = VK_TRUE;
	depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencilState.depthBoundsTestEnable = VK_FALSE;
	depthStencilState.minDepthBounds = 0.0f; // Optional
	depthStencilState.maxDepthBounds = 1.0f; // Optional
	depthStencilState.stencilTestEnable = VK_FALSE;
	depthStencilState.front = {};	// Optional
	depthStencilState.back = {};		// Optional

	// Graphics pipeline
	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &depthStencilState;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = pipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	pipelineInfo.basePipelineIndex = -1;
	if (vkCreateGraphicsPipelines(
		this->device,
		VK_NULL_HANDLE,
		1,
		&pipelineInfo,
		nullptr,
		&this->graphicsPipeline) != VK_SUCCESS)
	{
		Log::error("Failed to create graphics pipeline.");
	}

	vkDestroyShaderModule(this->device, fragShaderModule, nullptr);
	vkDestroyShaderModule(this->device, vertShaderModule, nullptr);
}

void Renderer::createCommandPool()
{
	QueueFamilyIndices queueFamilyIndices =
		findQueueFamilies(this->physicalDevice);

	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily.value();
	if (vkCreateCommandPool(
		this->device,
		&poolInfo,
		nullptr,
		&this->commandPool) != VK_SUCCESS)
	{
		Log::error("Failed to create command pool.");
	}
}

void Renderer::createFramebuffers()
{
	// Create one framebuffer for each swapchain image view
	this->swapChainFramebuffers.resize(this->swapChainImageViews.size());
	for (size_t i = 0; i < this->swapChainImageViews.size(); ++i)
	{
		std::array<VkImageView, 2> attachments =
		{
			this->swapChainImageViews[i],
			this->depthTexture.getImageView()
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = renderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(
			this->device,
			&framebufferInfo,
			nullptr,
			&this->swapChainFramebuffers[i]) != VK_SUCCESS)
		{
			Log::error("Failed to create framebuffer.");
		}
	}
}

void Renderer::createVertexBuffer()
{
	VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

	// Create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	this->createBuffer(
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
		this->device,
		stagingBufferMemory,
		0,
		bufferSize,
		0,
		&data
	);

	// Copy data to memory
	memcpy(data, vertices.data(), (size_t)bufferSize);

	// Unmap buffer memory
	vkUnmapMemory(this->device, stagingBufferMemory);

	// Create vertex buffer
	this->createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		this->vertexBuffer,
		this->vertexBufferMemory
	);

	// Copy from staging buffer to real buffer
	this->copyBuffer(stagingBuffer, this->vertexBuffer, bufferSize);

	// Deallocate staging buffer
	vkDestroyBuffer(this->device, stagingBuffer, nullptr);
	vkFreeMemory(this->device, stagingBufferMemory, nullptr);
}

void Renderer::createIndexBuffer()
{
	VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();

	// Create staging buffer
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;
	this->createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		stagingBuffer,
		stagingBufferMemory
	);

	// Map memory
	void* data;
	vkMapMemory(this->device, stagingBufferMemory, 0, bufferSize, 0, &data);
	memcpy(data, indices.data(), (size_t)bufferSize);
	vkUnmapMemory(this->device, stagingBufferMemory);

	// Create real buffer
	this->createBuffer(
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
		indexBuffer,
		indexBufferMemory
	);

	// Copy from staging buffer to real buffer
	this->copyBuffer(stagingBuffer, this->indexBuffer, bufferSize);

	// Deallocate staging buffer
	vkDestroyBuffer(this->device, stagingBuffer, nullptr);
	vkFreeMemory(this->device, stagingBufferMemory, nullptr);
}

void Renderer::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	this->uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	this->uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		this->createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			this->uniformBuffers[i],
			this->uniformBuffersMemory[i]
		);
	}
}

void Renderer::createDescriptorPool()
{
	// Pool size
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	// Create descriptor pool
	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	poolInfo.flags = 0;
	if (vkCreateDescriptorPool(
		this->device,
		&poolInfo,
		nullptr,
		&this->descriptorPool) != VK_SUCCESS)
	{
		Log::error("Failed to create descriptor pool.");
	}
}

void Renderer::createDescriptorSets()
{
	this->descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

	// Allocate descriptor sets
	std::vector<VkDescriptorSetLayout> layouts(
		MAX_FRAMES_IN_FLIGHT,
		this->descriptorSetLayout
	);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = this->descriptorPool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	allocInfo.pSetLayouts = layouts.data();
	if (vkAllocateDescriptorSets(this->device, &allocInfo, this->descriptorSets.data()) != VK_SUCCESS)
	{
		Log::error("Failed to allocate descriptor sets.");
	}

	// Populate descriptor sets
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		// Descriptor buffer info
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = this->uniformBuffers[i];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformBufferObject);

		// Descriptor image info
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = this->texture.getImageView();
		imageInfo.sampler = this->texture.getSampler();

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

		// Write descriptor set for uniform buffer
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = this->descriptorSets[i];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &bufferInfo;

		// Write descriptor set for image sampler
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = this->descriptorSets[i];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pImageInfo = &imageInfo;

		vkUpdateDescriptorSets(
			this->device,
			static_cast<uint32_t>(descriptorWrites.size()),
			descriptorWrites.data(),
			0,
			nullptr
		);
	}
}

void Renderer::createCommandBuffers()
{
	this->commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

	// Allocate command buffer from command pool
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	if (vkAllocateCommandBuffers(
		this->device,
		&allocInfo,
		this->commandBuffers.data()) != VK_SUCCESS)
	{
		Log::error("Failed to allocate command buffers.");
	}
}

void Renderer::createSyncObjects()
{
	this->imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	this->renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	this->inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		if (vkCreateSemaphore(
			this->device,
			&semaphoreInfo,
			nullptr,
			&this->imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(
				this->device,
				&semaphoreInfo,
				nullptr,
				&this->renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(
				this->device,
				&fenceInfo,
				nullptr,
				&this->inFlightFences[i]
			) != VK_SUCCESS)
		{
			Log::error("Failed to create semaphores/fences.");
		}
	}
}

void Renderer::drawFrame()
{
	// Wait, then reset fence
	vkWaitForFences(this->device, 1, &this->inFlightFences[this->currentFrame], VK_TRUE, UINT64_MAX);

	// Get next image index from the swapchain
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		this->device,
		this->swapChain,
		UINT64_MAX,
		this->imageAvailableSemaphores[this->currentFrame],
		VK_NULL_HANDLE,
		&imageIndex
	);

	// Window resize?
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		this->recreateSwapChain();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		Log::error("Failed to acquire swapchain image.");
	}

	this->updateUniformBuffer(this->currentFrame);

	// Only reset the fence if we are submitting work
	vkResetFences(this->device, 1, &this->inFlightFences[this->currentFrame]);

	// Reset command buffer
	vkResetCommandBuffer(this->commandBuffers[this->currentFrame], 0);

	// Record command buffer
	this->recordCommandBuffer(this->commandBuffers[this->currentFrame], imageIndex);

	// Info for submitting command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { this->imageAvailableSemaphores[this->currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &this->commandBuffers[this->currentFrame];

	VkSemaphore signalSemaphores[] = { this->renderFinishedSemaphores[this->currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// Submit command buffer
	if (vkQueueSubmit(
		this->graphicsQueue,
		1,
		&submitInfo,
		this->inFlightFences[this->currentFrame])
		!= VK_SUCCESS)
	{
		Log::error("Failed to submit draw command buffer.");
	}

	// Present info
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { this->swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	// Present!
	result = vkQueuePresentKHR(this->presentQueue, &presentInfo);

	// Window resize?
	if (result == VK_ERROR_OUT_OF_DATE_KHR ||
		result == VK_SUBOPTIMAL_KHR ||
		this->framebufferResized)
	{
		this->framebufferResized = false;
		this->recreateSwapChain();
	}
	else if (result != VK_SUCCESS)
	{
		Log::error("Failed to present swapchain image.");
	}

	// Next frame
	this->currentFrame = (this->currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::updateUniformBuffer(uint32_t currentImage)
{
	// Delta time
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(
		currentTime - startTime
		).count();

	// Create ubo struct with matrix data
	UniformBufferObject ubo{};
	ubo.model = glm::rotate(
		glm::mat4(1.0f),
		time * glm::radians(90.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	ubo.view = glm::lookAt(
		glm::vec3(2.0f, 2.0f, 2.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f)
	);
	ubo.proj = glm::perspective(
		glm::radians(45.0f),
		swapChainExtent.width / (float)swapChainExtent.height,
		0.1f,
		10.0f
	);
	ubo.proj[1][1] *= -1.0f;

	// Copy data to uniform buffer object
	void* data;
	vkMapMemory(
		this->device,
		this->uniformBuffersMemory[currentImage],
		0,
		sizeof(ubo),
		0,
		&data
	);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(this->device, this->uniformBuffersMemory[currentImage]);
}

void Renderer::cleanupSwapChain()
{
	this->depthTexture.cleanup();

	for (auto framebuffer : swapChainFramebuffers)
		vkDestroyFramebuffer(this->device, framebuffer, nullptr);

	for (auto imageView : swapChainImageViews)
		vkDestroyImageView(this->device, imageView, nullptr);

	vkDestroySwapchainKHR(this->device, this->swapChain, nullptr);
}

void Renderer::recreateSwapChain()
{
	// Handle minimization when width/height is 0
	int width = 0, height = 0;
	glfwGetFramebufferSize(this->window->getWindowHandle(), &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(this->window->getWindowHandle(), &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(this->device);

	this->cleanupSwapChain();

	this->createSwapChain();
	this->createImageViews();
	this->depthTexture.createAsDepthTexture(
		this->swapChainExtent.width,
		this->swapChainExtent.height
	);
	this->createFramebuffers();
}

void Renderer::populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = /*VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |*/
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
		VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
}

VkSurfaceFormatKHR Renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	// Find specific format/color space combination
	for (const auto& availableFormat : availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	// Use the first format/color space combination
	Log::warning("First surface format was chosen.");
	return availableFormats[0];
}

VkPresentModeKHR Renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
{
	// Find specific present mode
	for (const auto& availablePresentMode : availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
			return availablePresentMode;
	}

	// Guaranteed to be available
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D Renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != ~uint32_t(0))
	{
		return capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(this->window->getWindowHandle(), &width, &height);

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(
			actualExtent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		actualExtent.height = std::clamp(
			actualExtent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);

		return actualExtent;
	}
}

SwapChainSupportDetails Renderer::querySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;

	// Capabilities
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		device, this->surface, &details.capabilities
	);

	// Formats
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(
		device, this->surface, &formatCount, nullptr
	);
	if (formatCount != 0)
	{
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(
			device, this->surface, &formatCount, details.formats.data()
		);
	}

	// Presentation modes
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(
		device, this->surface,
		&presentModeCount, nullptr
	);
	if (presentModeCount != 0)
	{
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(
			device, this->surface,
			&presentModeCount, details.presentModes.data()
		);
	}

	return details;
}

VkShaderModule Renderer::createShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(this->device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		Log::error("Failed to create shader module.");
	}

	return shaderModule;
}

std::vector<const char*> Renderer::getRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}

bool Renderer::checkValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;

		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}

	return true;
}

bool Renderer::checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	// Get available extensions
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(
		device, nullptr,
		&extensionCount, nullptr
	);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(
		device, nullptr,
		&extensionCount, availableExtensions.data()
	);

	// Unique required extensions
	std::set<std::string> requiredExtensions(
		deviceExtensions.begin(), deviceExtensions.end()
	);

	// Remove found extensions
	for (const auto& extension : availableExtensions)
		requiredExtensions.erase(extension.extensionName);

	// Have all required extensions been found and removed?
	return requiredExtensions.empty();
}

bool Renderer::isDeviceSuitable(VkPhysicalDevice device)
{
	// Find queue families
	QueueFamilyIndices indices = findQueueFamilies(device);

	// Find required extension support
	bool extensionsSupported = checkDeviceExtensionSupport(device);

	// Swap chain with correct support
	bool swapChainAdequate = false;
	if (extensionsSupported)
	{
		SwapChainSupportDetails swapChainSupport =
			querySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.formats.empty() &&
			!swapChainSupport.presentModes.empty();
	}

	// Sampler anisotropy support
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);

	return indices.isComplete() &&
		extensionsSupported &&
		swapChainAdequate &&
		supportedFeatures.samplerAnisotropy;
}

QueueFamilyIndices Renderer::findQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;

	// Get queue family handles
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	// Find indices
	int i = 0;
	for (const auto& queueFamily : queueFamilies)
	{
		// Graphics queue family
		if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
			indices.graphicsFamily = i;

		// Present queue family
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(
			device,
			i,
			this->surface,
			&presentSupport
		);
		if (presentSupport)
			indices.presentFamily = i;

		// Done
		if (indices.isComplete())
			break;

		i++;
	}

	return indices;
}

bool Renderer::hasStencilComponent(VkFormat format)
{
	return format == VK_FORMAT_D32_SFLOAT_S8_UINT ||
		format == VK_FORMAT_D24_UNORM_S8_UINT;
}

uint32_t Renderer::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(this->physicalDevice, &memProperties);

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

void Renderer::createBuffer(
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
		this->device,
		&bufferInfo,
		nullptr,
		&buffer) != VK_SUCCESS)
	{
		Log::error("Failed to create buffer.");
	}

	// Get memory requirements from buffer
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(
		this->device,
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
		this->device,
		&allocInfo,
		nullptr,
		&bufferMemory) != VK_SUCCESS)
	{
		Log::error("Failed to allocate vertex buffer memory.");
	}

	// Bind memory to buffer
	vkBindBufferMemory(
		this->device,
		buffer,
		bufferMemory,
		0
	);
}

void Renderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = this->beginSingleTimeCommands();

	// Record copy buffer
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

	this->endSingleTimeCommands(commandBuffer);
}

VkImageView Renderer::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags)
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
	if (vkCreateImageView(this->device, &viewInfo, nullptr, &imageView))
		Log::error("Failed to create texture image view.");

	return imageView;
}

VkCommandBuffer Renderer::beginSingleTimeCommands()
{
	VkCommandBuffer commandBuffer;

	// Allocate command buffer
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = this->commandPool;
	allocInfo.commandBufferCount = 1;
	vkAllocateCommandBuffers(this->device, &allocInfo, &commandBuffer);

	// Begin recording command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	return commandBuffer;
}

void Renderer::endSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	// End recording command buffer
	vkEndCommandBuffer(commandBuffer);

	// Submit command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(this->graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(this->graphicsQueue);

	// Free command buffer
	vkFreeCommandBuffers(this->device, this->commandPool, 1, &commandBuffer);
}

void Renderer::recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
	// Reset and begin recording into command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0;
	beginInfo.pInheritanceInfo = nullptr;
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		Log::error("Failed to begin recording command buffer.");
	}

	// Begin render pass
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = this->renderPass;
	renderPassInfo.framebuffer = this->swapChainFramebuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = this->swapChainExtent;

	// Clear values, for color and depth
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	// Record beginning render pass
	vkCmdBeginRenderPass(
		commandBuffer,
		&renderPassInfo,
		VK_SUBPASS_CONTENTS_INLINE
	);

	// Record binding graphics pipeline
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		this->graphicsPipeline
	);

	// Record dynamic viewport
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(swapChainExtent.width);
	viewport.height = static_cast<float>(swapChainExtent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

	// Record dynamic scissor
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = this->swapChainExtent;
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	// Record binding vertex/index buffer
	VkBuffer vertexBuffers[] = { this->vertexBuffer };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
	vkCmdBindIndexBuffer(commandBuffer, this->indexBuffer, 0, VK_INDEX_TYPE_UINT16);

	// Record binding descriptor sets
	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		this->pipelineLayout,
		0,
		1,
		&this->descriptorSets[this->currentFrame],
		0,
		nullptr
	);

	// Record draw!
	vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

	// Record ending render pass
	vkCmdEndRenderPass(commandBuffer);

	// Finish recording
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
	{
		Log::error("Failed to record command buffer.");
	}
}

Renderer::Renderer()
	: window(nullptr),
	depthTexture(*this),
	texture(*this),

	commandPool(VK_NULL_HANDLE),
	debugMessenger(VK_NULL_HANDLE)
{
}

Renderer::~Renderer()
{
}

void Renderer::init()
{
	this->initVulkan();
}

void Renderer::setWindow(Window& window)
{
	this->window = &window;
}
