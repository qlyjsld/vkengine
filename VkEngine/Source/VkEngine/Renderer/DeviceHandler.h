#pragma once

#include <vulkan/vulkan.h>
#include <optional>

namespace VkEngine
{

	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphicFamily;
		std::optional<uint32_t> presentFamily;

		bool isComplete()
		{
			return graphicFamily.has_value() && presentFamily.has_value();
		}
	};

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

		QueueFamilyIndices _indices;

		inline VkDevice getDevice() { return _device; };
		inline VkPhysicalDevice getPhysicalDevice() { return _physicalDevice; };

		inline QueueFamilyIndices getIndices() { return _indices; };

		void init(VkInstance intance);
		void release();
	};
}

