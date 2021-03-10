#include <iostream>
#include <stdexcept>
#include <cstdlib>

#include "vk_engine.h"

int main(){
	vk_engine engine{};

	try {
		engine.run();
	}
	catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}