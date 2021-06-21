#include <iostream>
#include <stdexcept>

#include "VkEngine.h"
#include "QuakeStyleConsole.h"

int main()
{
	// Virtuoso::QuakeStyleConsole console;

	try
	{
		VkEngine::Renderer engine{};
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
