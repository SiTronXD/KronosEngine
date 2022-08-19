#include "Engine.h"
#include "Application/Time.h"
#include "Graphics/Mesh.h"

void Engine::loadMesh(Mesh& outputMesh)
{
	std::vector<Vertex> vertices =
	{
		{{  0.5f,  0.0f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
		{{ -0.5f,  0.0f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
		{{ -0.5f,  0.0f,  0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }},
		{{  0.5f,  0.0f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},

		{{  0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f, 0.0f }, { 1.0f, 0.0f }},
		{{ -0.5f, -0.5f, -0.5f }, { 1.0f, 0.0f, 0.0f }, { 0.0f, 0.0f }},
		{{ -0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f, 1.0f }, { 0.0f, 1.0f }},
		{{  0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},
	};
	std::vector<uint32_t> indices =
	{
		0, 1, 2,
		2, 3, 0,

		4, 5, 6,
		6, 7, 4,
	};
	outputMesh.createMesh(vertices, indices);
}

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
	Mesh mesh(this->renderer);
	this->loadMesh(mesh);

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
		// TODO: change to scene submission rather
		// than mesh submission
		this->renderer.drawFrame(camera, mesh);

		// Print fps
		if (Time::hasOneSecondPassed())
			Log::write("FPS: " + std::to_string(1.0f / Time::getDT()));
	}

	// Cleanup
	this->renderer.startCleanup();
	mesh.cleanup();
	this->renderer.cleanup();
}
