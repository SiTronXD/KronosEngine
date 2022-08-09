#pragma once

#include <GLFW/glfw3.h>

#define GLFW_MAX_NUM_KEYS (GLFW_KEY_LAST + 1)

class Input
{
private:
	static bool keyDown[GLFW_MAX_NUM_KEYS];

public:
	static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static inline bool isKeyDown(int glfwKeyCode) { return Input::keyDown[glfwKeyCode]; }
};