#include "Input.h"
#include "../Dev/Log.h"

bool Input::keyDown[GLFW_MAX_NUM_KEYS];

void Input::glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	Input::keyDown[key] = (action != GLFW_RELEASE);
}