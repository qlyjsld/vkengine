#pragma once
#include <iostream>

// vulkan related error detection macro
#define VK_CHECK(x)\
	do\
	{\
		VkResult err = x;\
		if (err){\
			std::cout << "Error: " << err << std::endl;\
			abort();\
		}\
	} while (0);