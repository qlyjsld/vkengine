#pragma once

#include <vulkan/vulkan.h>
#include "VkEngine/Renderer/DeletionQueue.h"

namespace VkEngine
{

	class DeviceHandler
	{
	public:

		DeviceHandler(VkInstance intance)
		{
			init(intance);
		}

		~DeviceHandler() 
		{
			release();
		};

		VkDevice _device;
		VkPhysicalDevice _physicalDevice;
		VkPhysicalDeviceProperties _deviceProperties;

		DeletionQueue _deletionQueue;

		void init(VkInstance intance);
		void release();
	};
}

