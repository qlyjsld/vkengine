#pragma once

#include <vulkan/vulkan.h>

namespace VkEngine
{

	class SurfaceHandler
	{
	public:

		SurfaceHandler(VkInstance instance)
		{
			init(instance);
		}

		~SurfaceHandler()
		{
			release();
		};

		struct GLFWwindow* _window{ nullptr };

		VkSurfaceKHR _surface;

		inline VkSurfaceKHR getSurface() { return _surface; };

		void init();
		void release();
	};
}