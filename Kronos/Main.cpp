#include <stdexcept>
#include "Engine/Engine.h"

int main()
{
	// Set flags for tracking CPU memory leaks
	#ifdef _DEBUG
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	#endif // _DEBUG

	// Create engine within it's own scope
	{
		Engine engine;
		engine.init();
	}

	getchar();

	return EXIT_SUCCESS;
}