#include "Engine.h"

Engine::Engine()
{
}

Engine::~Engine()
{
}

void Engine::init()
{
	// Init
	this->window.init(this->renderer, 1280, 720);
	this->renderer.init();

	// Main loop
	while (this->window.isRunning())
	{
		this->window.update();
		this->renderer.drawFrame();
	}

	// Cleanup
	this->renderer.cleanup();
}
