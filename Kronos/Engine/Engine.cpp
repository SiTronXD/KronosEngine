#include "Engine.h"
#include "Application/Time.h"

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
	Time::init();
	while (this->window.isRunning())
	{
		// Update before "game logic"
		this->window.update();
		Time::updateDeltaTime();

		// "Game logic"
		camera.update();

		// Render
		this->renderer.drawFrame(camera);

		// Print fps
		if (Time::hasOneSecondPassed())
			Log::write("FPS: " + std::to_string(1.0f / Time::getDT()));
	}

	// Cleanup
	this->renderer.cleanup();
}
