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
	ImGui::Text("Current depth mode:");
	ImGui::Text(this->depthModeNames[(int)this->currentDepthMode].c_str());
	ImGui::Text("");

	// Depth modes
	ImGui::Text("Depth modes:");

	// Loop through all buttons
	DepthMode chosenDepthMode = DepthMode::NONE;
	for (size_t i = 0; i < this->depthModeNames.size(); ++i)
	{
		// Button was pressed, choose this mode
		if (ImGui::Button(this->depthModeNames[i].c_str()))
		{
			chosenDepthMode = (DepthMode) i;
			i = this->depthModeNames.size();
		}
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

		case DepthMode::BSP_FRONT_TO_BACK:
			this->bsp.setTraversalMode(BspTraversalMode::FRONT_TO_BACK);

			break;

		case DepthMode::BSP_FRONT_TO_BACK_WITH_STENCIL:
			this->bsp.setTraversalMode(BspTraversalMode::FRONT_TO_BACK);

			break;

		default:
			break;
		}

		this->currentDepthMode = chosenDepthMode;

		// Apply settings
		this->useBsp = 
			this->currentDepthMode == DepthMode::BSP_BACK_TO_FRONT ||
			this->currentDepthMode == DepthMode::BSP_FRONT_TO_BACK ||
			this->currentDepthMode == DepthMode::BSP_FRONT_TO_BACK_WITH_STENCIL;
		this->renderer.setDepthStencil(
			this->currentDepthMode == DepthMode::ONLY_DEPTH_TESTING,
			this->currentDepthMode == DepthMode::BSP_FRONT_TO_BACK_WITH_STENCIL
		);
	}

	// Update settings
	if (lastRenderWireframe != this->renderWireframe)
	{
		this->renderer.setToWireframe(this->renderWireframe);
	}
}

Engine::Engine()
	: currentDepthMode(DepthMode::BSP_BACK_TO_FRONT), 
	renderWireframe(false),
	useBsp(true)
{
	this->depthModeNames.push_back("BSP back-to-front traversal");
	this->depthModeNames.push_back("BSP front-to-back traversal");
	this->depthModeNames.push_back("BSP front-to-back traversal using stencil buffer");
	this->depthModeNames.push_back("Standard depth testing (with BSP-split mesh)");
	this->depthModeNames.push_back("Ignore depth");
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

		this->updateImgui();

		// ------------- "Game logic" -------------
		camera.update();

		if (this->useBsp)
		{
			// Traverse BSP tree
			this->bsp.traverseTree(meshData, camera.getPosition());

			// Update index buffer from the tree traversal
			mesh.getIndexBuffer().updateIndexBuffer(
				meshData.getIndices(),
				this->renderer.getCurrentFrameIndex()
			);
		}
		// ------------- End of "game logic" -------------
		
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
