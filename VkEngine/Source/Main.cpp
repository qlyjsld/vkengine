#include <iostream>
#include <stdexcept>

#include "VkEngine.h"
#include "VkEngine/Core/ConsoleVariableSystem.h"

int main()
{
	// Virtuoso::QuakeStyleConsole console;

	try
	{
		VkEngine::Renderer renderer();
	}
	
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
