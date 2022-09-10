#include "Engine.h"
#include "Application/Time.h"
#include "Application/Input.h"
#include "Graphics/Mesh.h"
#include "DataStructures/BSP.h"

void Engine::updateImgui()
{
	// Start
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Window
	bool lastRenderWireframe = this->renderWireframe;
	ImGui::Begin("Settings");

	// Wireframe
	ImGui::Checkbox("Wireframe", &this->renderWireframe);

	// Triangle depth mode
	static bool b1;
	static bool b2;
	static bool b3;
	static bool b4;
	static bool b5;
	if (ImGui::BeginMenu("Depth mode"))
	{
		b1 = ImGui::Button("BSP back-to-front traversal");
		b2 = ImGui::Button("BSP front-to-back traversal");
		b3 = ImGui::Button("BSP front-to-back traversal with stencil masking");
		b4 = ImGui::Button("Standard depth testing (with BSP-split mesh)");
		b5 = ImGui::Button("None");
		ImGui::EndMenu();
	}

	if(b1 || b2 || b3 || b4 || b5)
		Log::write(
			std::to_string(b1) + 
			std::to_string(b2) + 
			std::to_string(b3) + 
			std::to_string(b4) + 
			std::to_string(b5));

	// End
	ImGui::End();
	ImGui::Render();

	// Update settings
	if (lastRenderWireframe != this->renderWireframe)
	{
		this->renderer.setToWireframe(this->renderWireframe);
	}
}

Engine::Engine()
	: renderWireframe(false)
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


		this->updateImgui();

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
