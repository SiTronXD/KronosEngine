#pragma once

#include "DataStructures/BSP.h"
#include "Graphics/Renderer.h"

enum class DepthMode
{
	BSP_BACK_TO_FRONT,
	BSP_FRONT_TO_BACK_NO_STENCIL,

	NONE,
};

class Engine
{
private:
	Window window;
	Renderer renderer;

	BSP bsp;

	std::vector<std::string> depthModeNames;

	DepthMode currentDepthMode;

	bool renderWireframe;

	void updateImgui();

public:
	Engine();
	~Engine();

	void init();
};