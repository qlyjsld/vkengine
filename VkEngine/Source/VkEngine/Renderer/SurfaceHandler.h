#pragma once

#include <vulkan/vulkan.h>

namespace VkEngine
{

	class SurfaceHandler
	{
	public:

		SurfaceHandler(VkInstance instance);
		~SurfaceHandler();

		struct GLFWwindow* _window{ nullptr };

		VkSurfaceKHR _surface;

		inline VkSurfaceKHR getSurface() { return _surface; };
	};
}