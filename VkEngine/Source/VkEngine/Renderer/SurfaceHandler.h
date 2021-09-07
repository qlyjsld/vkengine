#pragma once

#include <vulkan/vulkan.h>

struct GLFWwindow;

namespace VkEngine
{

	class SurfaceHandler
	{
	public:

		SurfaceHandler(VkInstance instance);
		~SurfaceHandler();

		inline VkSurfaceKHR getSurface() { return _surface; };
		inline GLFWwindow* getWindow() { return _window; };

	private:

		GLFWwindow* _window{ nullptr };

		VkSurfaceKHR _surface;
	};
}