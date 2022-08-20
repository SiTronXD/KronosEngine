#define FAST_OBJ_IMPLEMENTATION
#include <fast_obj.h>

#include "Engine.h"
#include "Application/Time.h"
#include "Application/Input.h"
#include "Graphics/Mesh.h"

/*
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
	4, 5, 6,
	6, 7, 4,

	0, 1, 2,
	2, 3, 0
};
*/

void Engine::loadMesh(Mesh& outputMesh)
{
	// Load model (assume triangulation)
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	fastObjMesh* loadedObj = fast_obj_read("Resources/Models/dragon_vrip_res4.obj");

	// Positions
	vertices.resize(loadedObj->index_count);
	for (unsigned int i = 0; i < loadedObj->position_count; ++i)
	{
		Vertex& v = vertices[i];
		v.pos.x = loadedObj->positions[i * 3 + 0];
		v.pos.y = loadedObj->positions[i * 3 + 1];
		v.pos.z = loadedObj->positions[i * 3 + 2];

		v.color.x = 0.5f;
	}

	// Indices
	indices.resize(loadedObj->index_count);
	for (unsigned int i = 0; i < loadedObj->index_count; ++i)
	{
		indices[i] = loadedObj->indices[i].p;
	}

	// Visualize normals as colors
	for (size_t i = 0; i < indices.size(); i += 3)
	{
		Vertex& v0 = vertices[indices[i + 0]];
		Vertex& v1 = vertices[indices[i + 1]];
		Vertex& v2 = vertices[indices[i + 2]];

		glm::vec3 edge0 = v1.pos - v0.pos;
		glm::vec3 edge1 = v2.pos - v0.pos;

		glm::vec3 normal = glm::cross(edge0, edge1);
		normal = glm::normalize(normal);

		v0.color += normal;
		v1.color += normal;
		v2.color += normal;
	}

	// Normalize smooth normals
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		Vertex& v = vertices[i];
		if (glm::dot(v.color, v.color) >= 0.01f)
			v.color = glm::normalize(v.color);
	}


	// Create mesh
	outputMesh.createMesh(vertices, indices, true);


	fast_obj_destroy(loadedObj);
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
