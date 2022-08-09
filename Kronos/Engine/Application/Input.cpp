#include "Input.h"
#include "../Dev/Log.h"

void Input::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Log::write("Key callback!");
}
