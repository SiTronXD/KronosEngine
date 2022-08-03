#include "Window.h"

#include "../Graphics/Renderer.h"

static void framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto app = reinterpret_cast<Renderer*>(
		glfwGetWindowUserPointer(window)
		);
	app->framebufferResized = true;
}

Window::Window()
	: windowHandle(nullptr)
{
}

Window::~Window()
{
	// GLFW
	glfwDestroyWindow(this->windowHandle);
	glfwTerminate();
}
#include "../Dev/Log.h"
void Window::init(Renderer& renderer, int width, int height)
{
	// Set pointer
	renderer.setWindow(*this);

	// Create window
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	this->windowHandle = glfwCreateWindow(width, height, "Vulkan", nullptr, nullptr);

	// Set position to center of monitor
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	glfwSetWindowPos(
		this->windowHandle, 
		(mode->width - width) / 2, 
		(mode->height - height) / 2
	);

	// Set pointer for resize callback
	glfwSetWindowUserPointer(this->windowHandle, &renderer);
	glfwSetFramebufferSizeCallback(this->windowHandle, framebufferResizeCallback);
}

void Window::update()
{
	glfwPollEvents();
}

bool Window::isRunning() const
{
	return !glfwWindowShouldClose(this->windowHandle);
}
