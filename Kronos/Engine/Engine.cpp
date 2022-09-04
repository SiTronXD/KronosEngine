#include "Engine.h"
#include "Application/Time.h"
#include "Application/Input.h"
#include "Graphics/Mesh.h"
#include "DataStructures/BSP.h"

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

	// Mesh data to render
	MeshData meshData;
	//meshData.loadOBJ("Resources/Models/dragon_vrip_res4.obj");
	//meshData.loadOBJ("Resources/Models/dragon_vrip_res4_big.obj");
	meshData.loadOBJ("Resources/Models/suzanne.obj");
	//meshData.loadOBJ("Resources/Models/sphereTest.obj");
	//meshData.loadOBJ("Resources/Models/lowResSphere.obj");
	//meshData.loadOBJ("Resources/Models/lowResThreeSpheres.obj");
	//meshData.loadOBJ("Resources/Models/torus.obj");

	// BSP to render mesh with
	BSP bsp;
	bsp.createFromMeshData(meshData);

	// Randomly create trees and choose the best one
	/*BSP* bsp = nullptr;
	BSP bsps[2]{};
	uint32_t currentIndex = 0;
	uint32_t lowestDepth = ~0u;
	for (uint32_t i = 0; i < 40; ++i)
	{
		MeshData tempMeshData; 
		tempMeshData.loadOBJ("Resources/Models/dragon_vrip_res4.obj");
		bsps[currentIndex].createFromMeshData(tempMeshData);

		uint32_t treeDepth = bsps[currentIndex].getTreeDepth();
		if (treeDepth < lowestDepth)
		{
			Log::write("Switched BSP");

			lowestDepth = treeDepth;
			bsp = &bsps[currentIndex];
			meshData = tempMeshData;
			currentIndex = (currentIndex + 1) % 2;
		}
	}
	Log::write("Chosen tree depth: " + std::to_string(lowestDepth));*/


	// Mesh to render
	Mesh mesh(this->renderer);
	mesh.createMesh(meshData, true);

	bool wireframe = false;

	// Main loop
	Time::init();
	while (this->window.isRunning())
	{
		// Update before "game logic"
		this->window.update();
		Time::updateDeltaTime();

		// "Game logic"
		camera.update();
		bsp.traverseTree(meshData, camera.getPosition());
		mesh.getIndexBuffer().updateIndexBuffer(
			meshData.getIndices(), 
			this->renderer.getCurrentFrameIndex()
		);

		if (Input::isKeyPressed(Keys::R))
		{
			wireframe = !wireframe;
			renderer.setToWireframe(wireframe);
		}

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
