#pragma once

#include <GLFW/glfw3.h>

#define GLFW_MAX_NUM_KEYS (GLFW_KEY_LAST + 1)

enum class Keys
{
	A = GLFW_KEY_A,
	B = GLFW_KEY_B,
	C = GLFW_KEY_C,
	D = GLFW_KEY_D,
	E = GLFW_KEY_E,
	F = GLFW_KEY_F,
	G = GLFW_KEY_G,
	H = GLFW_KEY_H,
	I = GLFW_KEY_I,
	J = GLFW_KEY_J,
	K = GLFW_KEY_K,
	L = GLFW_KEY_L,
	M = GLFW_KEY_M,
	N = GLFW_KEY_N,
	O = GLFW_KEY_O,
	P = GLFW_KEY_P,
	Q = GLFW_KEY_Q,
	R = GLFW_KEY_R,
	S = GLFW_KEY_S,
	T = GLFW_KEY_T,
	U = GLFW_KEY_U,
	V = GLFW_KEY_V,
	W = GLFW_KEY_W,
	X = GLFW_KEY_X,
	Y = GLFW_KEY_Y,
	Z = GLFW_KEY_Z,

	ESCAPE = GLFW_KEY_ESCAPE,
};

class Input
{
private:
	static bool keyDown[GLFW_MAX_NUM_KEYS];

public:
	static void glfwKeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static inline bool isKeyDown(const Keys& key) { return Input::keyDown[(int) key]; }
};