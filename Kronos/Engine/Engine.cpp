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
		this->currentMesh = this->useBsp ? &this->bspMesh : &this->originalMesh;
	}

	// Update settings
	if (lastRenderWireframe != this->renderWireframe)
	{
		this->renderer.setToWireframe(this->renderWireframe);
	}
}

Engine::Engine()
	: 
	originalMesh(this->renderer),
	bspMesh(this->renderer),
	currentMesh(nullptr),
	currentDepthMode(DepthMode::BSP_BACK_TO_FRONT), 
	renderWireframe(false),
	useBsp(true)
{
	this->depthModeNames.push_back("BSP back-to-front traversal");
	this->depthModeNames.push_back("BSP front-to-back traversal");
	this->depthModeNames.push_back("BSP front-to-back traversal using stencil buffer");
	this->depthModeNames.push_back("Standard depth testing (with original mesh)");
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
	MeshData originalMeshData;
	//originalMeshData.loadOBJ("Resources/Models/dragon_vrip_res4.obj");
	//originalMeshData.loadOBJ("Resources/Models/dragon_vrip_res4_big.obj");
	originalMeshData.loadOBJ("Resources/Models/suzanne.obj");
	//originalMeshData.loadOBJ("Resources/Models/sphereTest.obj");
	//originalMeshData.loadOBJ("Resources/Models/lowResSphere.obj");
	//originalMeshData.loadOBJ("Resources/Models/lowResThreeSpheres.obj");
	//originalMeshData.loadOBJ("Resources/Models/torus.obj");

	// Make a copy of the mesh data when BSP splitting
	MeshData bspMeshData(originalMeshData);

	// BSP to render mesh with
	this->bsp.createFromMeshData(bspMeshData);

	// Mesh to render
	this->originalMesh.createMesh(originalMeshData, false);
	this->bspMesh.createMesh(bspMeshData, true);
	this->currentMesh = &bspMesh;

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
			this->bsp.traverseTree(bspMeshData, camera.getPosition());

			// Update index buffer from the tree traversal
			this->bspMesh.getIndexBuffer().updateIndexBuffer(
				bspMeshData.getIndices(),
				this->renderer.getCurrentFrameIndex()
			);
		}
		// ------------- End of "game logic" -------------
		
		// Render
		// This should be changed to scene submission rather
		// than mesh submission
		this->renderer.draw(camera, *this->currentMesh);

		// Print fps
		if (Time::hasOneSecondPassed())
			Log::write("FPS: " + std::to_string(1.0f / Time::getDT()));
	}

	// Cleanup
	this->renderer.startCleanup();

	this->originalMesh.cleanup();
	this->bspMesh.cleanup();

	this->renderer.cleanup();
}
