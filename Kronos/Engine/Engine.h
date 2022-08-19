#pragma once

#include "Graphics/Renderer.h"

class Engine
{
private:
	Window window;
	Renderer renderer;

	void loadMesh(Mesh& outputMesh);

public:
	Engine();
	~Engine();

	void init();
};