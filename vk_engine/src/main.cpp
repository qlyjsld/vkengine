#include <iostream>
#include <stdexcept>

#include "vk_engine.h"
#include "QuakeStyleConsole.h"

int main()
{
	// Virtuoso::QuakeStyleConsole console;
	vk_engine::vk_renderer engine{};

	try
	{
		engine.run();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
