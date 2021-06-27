#pragma once

#include "VkEngine/Renderer/DeletionQueue.h"

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

		DeletionQueue _deletionQueue;

		void init();
		void release();
	};
}