#pragma once

#include "Graphics/Renderer.h"

class Engine
{
private:
	Renderer renderer;

public:
	Engine();
	~Engine();

	void run();
};