#include "Renderer.h"
#include "Vulkan/SupportChecker.h"

#include <iostream>
#include <string>
#include <set>
#include <chrono>

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> instanceExtensions =
{
#ifdef _DEBUG
	"VK_EXT_validation_features"
#endif
};

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
const bool enablePrintingBestPractices = false;
#else
const bool enableValidationLayers = true;
const bool enablePrintingBestPractices = false;
#endif

void Renderer::initVulkan()
{
	this->instance.createInstance(
		enableValidationLayers, 
		enablePrintingBestPractices,
		instanceExtensions,
		validationLayers, 
		this->window
	);
	this->debugMessenger.createDebugMessenger(enableValidationLayers);
	this->surface.createSurface();
	this->physicalDevice.pickPhysicalDevice(
		this->instance,
		this->surface,
		deviceExtensions, 
		this->queueFamilies
	);
	this->device.createDevice(
		deviceExtensions, 
		validationLayers, 
		enableValidationLayers, 
		this->queueFamilies.getIndices()
	);
	this->queueFamilies.extractQueueHandles(this->getVkDevice());
	this->swapchain.createSwapchain();
	this->renderPass.createRenderPass();
	this->descriptorSetLayout.createDescriptorSetLayout();
	this->graphicsPipelineLayout.createPipelineLayout(this->descriptorSetLayout);
	this->graphicsPipeline.createGraphicsPipeline(
		this->graphicsPipelineLayout,
		this->renderPass
	);

	this->commandPool.create(VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT);
	this->singleTimeCommandPool.create(VK_COMMAND_POOL_CREATE_TRANSIENT_BIT);
	this->commandBuffers.createCommandBuffers(MAX_FRAMES_IN_FLIGHT);
	this->swapchain.createFramebuffers();

	// Shader resources
	this->texture.createFromFile("Resources/Textures/poggers.PNG");
	
	this->createUniformBuffers();
	
	this->descriptorPool.createDescriptorPool(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));
	this->descriptorSets.createDescriptorSets(
		this->descriptorSetLayout,
		this->descriptorPool,
		static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT),
		
		this->uniformBuffers,
		this->texture);

	this->createSyncObjects();




	// Imgui
	this->imguiRenderPass.createImguiRenderPass();
	this->imguiDescriptorPool.createImguiDescriptorPool(static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT));

	this->imguiFramebuffers.resize(this->swapchain.getImageCount());
	for (size_t i = 0; i < this->swapchain.getImageCount(); ++i)
	{
		std::array<VkImageView, 1> attachments =
		{
			this->swapchain.getImageView(i)
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = this->imguiRenderPass.getVkRenderPass();
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = this->swapchain.getWidth();
		framebufferInfo.height = this->swapchain.getHeight();
		framebufferInfo.layers = 1;
		if (vkCreateFramebuffer(
			this->getVkDevice(),
			&framebufferInfo,
			nullptr,
			&this->imguiFramebuffers[i]) != VK_SUCCESS)
		{
			Log::error("Failed to create imgui framebuffer.");
		}
	}
}

static void checkVkResult(VkResult err)
{
	if (err == 0)
		return;

	Log::error("Vulkan error: " + std::to_string(err));
}

void Renderer::initImgui()
{
	// Imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	this->imguiIO = &ImGui::GetIO(); (void)this->imguiIO;
	this->imguiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
	this->imguiIO->ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	this->imguiIO->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	ImGui::StyleColorsDark();

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (this->imguiIO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	// Change colors to approximately handle SRGB swapchain format
	for (uint32_t i = 0; i < ImGuiCol_COUNT; ++i)
	{
		style.Colors[i].x = std::pow(style.Colors[i].x, 2.2f);
		style.Colors[i].y = std::pow(style.Colors[i].y, 2.2f);
		style.Colors[i].z = std::pow(style.Colors[i].z, 2.2f);
	}

	ImGui_ImplGlfw_InitForVulkan(this->window->getWindowHandle(), true);
	ImGui_ImplVulkan_InitInfo initInfo = {};
	initInfo.Instance = this->getVkInstance();
	initInfo.PhysicalDevice = this->getVkPhysicalDevice();
	initInfo.Device = this->getVkDevice();
	initInfo.QueueFamily = this->queueFamilies.getIndices().graphicsFamily.value();
	initInfo.Queue = this->queueFamilies.getVkGraphicsQueue();
	initInfo.PipelineCache = VK_NULL_HANDLE;
	initInfo.DescriptorPool = this->imguiDescriptorPool.getVkDescriptorPool();
	initInfo.Allocator = nullptr;
	initInfo.MinImageCount = this->swapchain.getMinImageCount();
	initInfo.ImageCount = this->swapchain.getImageCount();
	initInfo.CheckVkResultFn = checkVkResult;
	ImGui_ImplVulkan_Init(&initInfo, this->imguiRenderPass.getVkRenderPass());

	// Upload font to gpu
	VkCommandBuffer tempCommandBuffer = CommandBuffer::beginSingleTimeCommands(*this);
	ImGui_ImplVulkan_CreateFontsTexture(tempCommandBuffer);
	CommandBuffer::endSingleTimeCommands(*this, tempCommandBuffer);

	ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void Renderer::cleanup()
{
	this->cleanupImgui();

	this->swapchain.cleanup();

	this->texture.cleanup();

	this->descriptorPool.cleanup();

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroyBuffer(this->getVkDevice(), this->uniformBuffers[i], nullptr);
		vkFreeMemory(this->getVkDevice(), this->uniformBuffersMemory[i], nullptr);
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		vkDestroySemaphore(this->getVkDevice(), this->imageAvailableSemaphores[i], nullptr);
		vkDestroySemaphore(this->getVkDevice(), this->renderFinishedSemaphores[i], nullptr);
		vkDestroyFence(this->getVkDevice(), this->inFlightFences[i], nullptr);
	}

	this->singleTimeCommandPool.cleanup();
	this->commandPool.cleanup();
	this->graphicsPipeline.cleanup();
	this->graphicsPipelineLayout.cleanup();
	this->descriptorSetLayout.cleanup();
	this->renderPass.cleanup();
	this->device.cleanup();
	this->debugMessenger.cleanup();
	this->surface.cleanup();

	// Destroys both physical device and instance
	this->instance.cleanup();
}

void Renderer::createUniformBuffers()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	this->uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
	this->uniformBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		Buffer::createBuffer(
			this->getVkPhysicalDevice(),
			this->getVkDevice(),
			bufferSize,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			this->uniformBuffers[i],
			this->uniformBuffersMemory[i]
		);
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
			this->getVkDevice(),
			&semaphoreInfo,
			nullptr,
			&this->imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(
				this->getVkDevice(),
				&semaphoreInfo,
				nullptr,
				&this->renderFinishedSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(
				this->getVkDevice(),
				&fenceInfo,
				nullptr,
				&this->inFlightFences[i]
			) != VK_SUCCESS)
		{
			Log::error("Failed to create semaphores/fences.");
		}
	}
}

void Renderer::drawFrame(Camera& camera, Mesh& mesh)
{
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGui::ShowDemoWindow();
	ImGui::Render();


	// Wait, then reset fence
	vkWaitForFences(
		this->getVkDevice(), 
		1, 
		&this->inFlightFences[this->currentFrameIndex],
		VK_TRUE, 
		UINT64_MAX
	);

	// Get next image index from the swapchain
	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(
		this->getVkDevice(),
		this->swapchain.getVkSwapchain(),
		UINT64_MAX,
		this->imageAvailableSemaphores[this->currentFrameIndex],
		VK_NULL_HANDLE,
		&imageIndex
	);

	// Window resize?
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		this->resizeWindow();
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		Log::error("Failed to acquire swapchain image.");
	}

	this->updateUniformBuffer(this->currentFrameIndex, camera);

	// Only reset the fence if we are submitting work
	vkResetFences(this->getVkDevice(), 1, &this->inFlightFences[this->currentFrameIndex]);

	ImDrawData* drawData = ImGui::GetDrawData();

	// Record command buffer
	this->recordCommandBuffer(imageIndex, mesh, drawData);

	// Update and Render additional Platform Windows
	if (this->imguiIO->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
	}


	// Info for submitting command buffer
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { this->imageAvailableSemaphores[this->currentFrameIndex] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = 
		&this->commandBuffers.getCommandBuffer(this->currentFrameIndex).getVkCommandBuffer();

	VkSemaphore signalSemaphores[] = { this->renderFinishedSemaphores[this->currentFrameIndex] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	// Submit command buffer
	if (vkQueueSubmit(
		this->queueFamilies.getVkGraphicsQueue(),
		1,
		&submitInfo,
		this->inFlightFences[this->currentFrameIndex])
		!= VK_SUCCESS)
	{
		Log::error("Failed to submit draw command buffer.");
	}

	// Present info
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { this->swapchain.getVkSwapchain() };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr;

	// Present!
	result = vkQueuePresentKHR(this->queueFamilies.getVkPresentQueue(), &presentInfo);

	// Window resize?
	if (result == VK_ERROR_OUT_OF_DATE_KHR ||
		result == VK_SUBOPTIMAL_KHR ||
		this->framebufferResized)
	{
		this->framebufferResized = false;
		this->resizeWindow();
	}
	else if (result != VK_SUCCESS)
	{
		Log::error("Failed to present swapchain image.");
	}

	// Next frame
	this->currentFrameIndex = (this->currentFrameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
}

void Renderer::setToWireframe(bool wireframe)
{
	vkDeviceWaitIdle(this->getVkDevice());

	this->graphicsPipeline.cleanup();
	this->graphicsPipeline.createGraphicsPipeline(
		this->graphicsPipelineLayout,
		this->renderPass,
		wireframe
	);
}

void Renderer::updateUniformBuffer(uint32_t currentImage, Camera& camera)
{
	// Delta time
	static auto startTime = std::chrono::high_resolution_clock::now();
	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(
		currentTime - startTime
		).count();

	// Create ubo struct with matrix data
	UniformBufferObject ubo{};
	/*ubo.model = glm::rotate(
		glm::mat4(1.0f),
		time * glm::radians(90.0f),
		glm::vec3(0.0f, 1.0f, 0.0f)
	);*/
	ubo.model = glm::mat4(1.0f);
	ubo.view = camera.getViewMatrix();
	ubo.proj = camera.getProjectionMatrix();

	// Copy data to uniform buffer object
	void* data;
	vkMapMemory(
		this->getVkDevice(),
		this->uniformBuffersMemory[currentImage],
		0,
		sizeof(ubo),
		0,
		&data
	);
	memcpy(data, &ubo, sizeof(ubo));
	vkUnmapMemory(this->getVkDevice(), this->uniformBuffersMemory[currentImage]);
}

void Renderer::recordCommandBuffer(
	uint32_t imageIndex, 
	Mesh& mesh,
	ImDrawData* drawData)
{
	CommandBuffer& commandBuffer = this->commandBuffers.getCommandBuffer(this->currentFrameIndex);

	// Begin
	commandBuffer.resetAndBegin();

		// Record dynamic viewport
		float swapchainHeight = (float)this->swapchain.getHeight();
		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = swapchainHeight;
		viewport.width = static_cast<float>(this->swapchain.getWidth());
		viewport.height = -swapchainHeight;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		commandBuffer.setViewport(viewport);

		// Record dynamic scissor
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = this->swapchain.getVkExtent();
		commandBuffer.setScissor(scissor);

		// Begin render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = this->renderPass.getVkRenderPass();
		renderPassInfo.framebuffer = this->swapchain.getVkFramebuffer(imageIndex);
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = this->swapchain.getVkExtent();

		// Clear values, for color and depth
		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 1.0f } };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		// Record beginning render pass
		commandBuffer.beginRenderPass(renderPassInfo);

			// Record binding graphics pipeline
			commandBuffer.bindPipeline(this->graphicsPipeline);

			// Record binding descriptor sets
			commandBuffer.bindDescriptorSet(
				this->graphicsPipelineLayout,
				this->descriptorSets.getDescriptorSet(this->currentFrameIndex)
			);

				// Record binding vertex/index buffer
				commandBuffer.bindVertexBuffer(mesh.getVertexBuffer());
				commandBuffer.bindIndexBuffer(mesh.getIndexBuffer(), this->currentFrameIndex);

				// Record draw!
				commandBuffer.drawIndexed(mesh.getNumIndices());

		// End render pass
		commandBuffer.endRenderPass();

		// Begin render pass
		VkRenderPassBeginInfo renderPassInfoImgui{};
		renderPassInfoImgui.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfoImgui.renderPass = this->imguiRenderPass.getVkRenderPass();
		renderPassInfoImgui.framebuffer = this->imguiFramebuffers[imageIndex];
		renderPassInfoImgui.renderArea.extent = this->swapchain.getVkExtent();
		renderPassInfoImgui.clearValueCount = 0;

		// Record beginning render pass
		commandBuffer.beginRenderPass(renderPassInfoImgui);

		// Record imgui primitives into it's command buffer
		ImGui_ImplVulkan_RenderDrawData(drawData, commandBuffer.getVkCommandBuffer());

		// End render pass
		commandBuffer.endRenderPass();

	// Stop recording
	commandBuffer.end();
}

void Renderer::resizeWindow()
{
	this->swapchain.recreate();

	ImGui_ImplVulkan_SetMinImageCount(this->swapchain.getMinImageCount());
}

void Renderer::cleanupImgui()
{
	for (auto framebuffer : this->imguiFramebuffers)
		vkDestroyFramebuffer(this->getVkDevice(), framebuffer, nullptr);

	this->imguiRenderPass.cleanup();

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	this->imguiDescriptorPool.cleanup();
}

Renderer::Renderer()
	: window(nullptr),

	texture(*this),

	debugMessenger(*this),
	surface(*this),
	device(*this),
	renderPass(*this),
	descriptorSetLayout(*this),
	graphicsPipelineLayout(*this),
	graphicsPipeline(*this),
	commandPool(*this),
	singleTimeCommandPool(*this),
	commandBuffers(*this, commandPool),
	descriptorPool(*this),
	descriptorSets(*this),
	swapchain(*this),

	imguiRenderPass(*this),
	imguiDescriptorPool(*this),
	imguiIO(nullptr)
{
}

Renderer::~Renderer()
{
}

void Renderer::init()
{
	this->initVulkan();
	this->initImgui();
}

void Renderer::setWindow(Window& window)
{
	this->window = &window;
}

void Renderer::startCleanup()
{
	// Wait for device before cleanup
	vkDeviceWaitIdle(this->getVkDevice());
}
