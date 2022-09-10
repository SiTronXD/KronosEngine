#pragma once

#include "Graphics/Renderer.h"

class Engine
{
private:
	Window window;
	Renderer renderer;

	bool renderWireframe;

	void updateImgui();

public:
	Engine();
	~Engine();

	void init();
};