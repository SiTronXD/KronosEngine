#include "Instance.h"

#include "SupportChecker.h"
#include "DebugMessenger.h"
#include "../../Dev/Log.h"
#include "../../Application/Window.h"

Instance::Instance()
	: instance(VK_NULL_HANDLE)
{
}

Instance::~Instance()
{
}

void Instance::createInstance(
	bool enableValidationLayers,
	const std::vector<const char*>& validationLayers,
	Window* window)
{
	if (enableValidationLayers && !SupportChecker::checkValidationLayerSupport(validationLayers))
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
	appInfo.apiVersion = VK_API_VERSION_1_3;

	// Instance create info
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	// Get and set extensions
	auto extensions = SupportChecker::getRequiredExtensions(*window, enableValidationLayers);
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
		DebugMessenger::populateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount = 0;

		createInfo.pNext = nullptr;
	}

	// Create instance
	if (vkCreateInstance(&createInfo, nullptr, &this->instance) != VK_SUCCESS)
	{
		Log::error("Failed to create instance.");
	}
}

void Instance::cleanup()
{
	// Destroys both physical device and instance
	vkDestroyInstance(this->instance, nullptr);
}
