#include <stdexcept>
#include "Engine/Engine.h"

int main()
{
	// Set flags for tracking CPU memory leaks
	#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif // _DEBUG

	Engine engine;
	engine.run();

	/*HelloTriangleApplication app;
	try 
	{
		//app.run();
	}
	catch (const std::exception & e)
	{
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}*/

	getchar();

	return EXIT_SUCCESS;
}