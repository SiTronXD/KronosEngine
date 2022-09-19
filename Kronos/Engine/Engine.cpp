#include "Engine.h"
#include "Application/Time.h"
#include "Application/Input.h"
#include "Graphics/Mesh.h"

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

	// Show current depth mode
	ImGui::Text("");
	ImGui::Text(("Current depth mode: " + this->depthModeNames[(int) this->currentDepthMode]).c_str());
	ImGui::Text("");

	// Triangle depth mode
	DepthMode chosenDepthMode = DepthMode::NONE;
	if (ImGui::BeginMenu("Depth mode"))
	{
		// Loop through all buttons
		for (size_t i = 0; i < this->depthModeNames.size(); ++i)
		{
			// Button was pressed, choose this mode
			if (ImGui::Button(this->depthModeNames[i].c_str()))
			{
				chosenDepthMode = (DepthMode) i;
				i = this->depthModeNames.size();
			}
		}

		ImGui::EndMenu();
	}

	// End
	ImGui::End();
	ImGui::Render();

	// Depth mode switch
	if (chosenDepthMode != DepthMode::NONE)
	{
		switch (chosenDepthMode)
		{
		case DepthMode::BSP_BACK_TO_FRONT:
			this->bsp.setTraversalMode(BspTraversalMode::BACK_TO_FRONT);

			break;
		case DepthMode::BSP_FRONT_TO_BACK_NO_STENCIL:
			this->bsp.setTraversalMode(BspTraversalMode::FRONT_TO_BACK);

			break;

		default:
			break;
		}

		this->currentDepthMode = chosenDepthMode;
	}

	// Update settings
	if (lastRenderWireframe != this->renderWireframe)
	{
		this->renderer.setToWireframe(this->renderWireframe);
	}
}

Engine::Engine()
	: currentDepthMode(DepthMode::BSP_BACK_TO_FRONT), renderWireframe(false)
{
	this->depthModeNames.push_back("BSP back-to-front traversal");
	this->depthModeNames.push_back("BSP front-to-back traversal");
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
	this->bsp.createFromMeshData(meshData);

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
		this->bsp.traverseTree(meshData, camera.getPosition());
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
