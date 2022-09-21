#pragma once

#include "DataStructures/BSP.h"
#include "Graphics/Renderer.h"

enum class DepthMode
{
	BSP_BACK_TO_FRONT,
	BSP_FRONT_TO_BACK,
	BSP_FRONT_TO_BACK_WITH_STENCIL,
	ONLY_DEPTH_TESTING,
	IGNORE_DEPTH,

	NONE,
};

class Engine
{
private:
	Window window;
	Renderer renderer;

	BSP bsp;

	Mesh originalMesh;
	Mesh bspMesh;
	Mesh* currentMesh;

	std::vector<std::string> depthModeNames;

	DepthMode currentDepthMode;

	bool renderWireframe;
	bool useBsp;

	void updateImgui();

public:
	Engine();
	~Engine();

	void init();
};