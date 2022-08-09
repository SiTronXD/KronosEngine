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
	Camera camera(this->renderer);

	// Main loop
	while (this->window.isRunning())
	{
		this->window.update();

		camera.update();

		this->renderer.drawFrame(camera);
	}

	// Cleanup
	this->renderer.cleanup();
}
