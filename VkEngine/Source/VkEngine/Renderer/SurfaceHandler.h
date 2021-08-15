#pragma once

#include <vulkan/vulkan.h>

namespace VkEngine
{

	class SurfaceHandler
	{
	public:

		SurfaceHandler(VkInstance instance);
		~SurfaceHandler();

		inline VkSurfaceKHR getSurface() { return _surface; };

	private:

		struct GLFWwindow* _window{ nullptr };

		VkSurfaceKHR _surface;
	};
}