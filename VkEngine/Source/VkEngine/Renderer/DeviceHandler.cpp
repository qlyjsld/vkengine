#include "VkEngine/Renderer/DeviceHandler.h"

#include <stdexcept>

namespace VkEngine
{

	void DeviceHandler::init(VkInstance instance)
	{
		// pick physical device
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

		if (deviceCount == 0)
		{
			throw std::runtime_error("no physical device available!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

		for (const auto& device : devices)
		{
			vkGetPhysicalDeviceProperties(device, &_deviceProperties);
			if (_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
			{
				_physicalDevice = device;
				break;
			}
		}

		if (_physicalDevice == VK_NULL_HANDLE)
		{
			throw std::runtime_error("no physical device available!");
		}

		// create logical device
		_indices = vk_support::findQueueFamilies(_physicalDevice, _surface);

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
		std::set<uint32_t> uniqueQueueFamilies = { _indices.graphicFamily.value(), _indices.presentFamily.value() };

		float queuePriority = 1.0f;
		for (uint32_t queueFamily : uniqueQueueFamilies)
		{
			VkDeviceQueueCreateInfo queueCreateInfo = vk_info::DeviceQueueCreateInfo(queueFamily, queuePriority);
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo deviceCreateInfo = vk_info::DeviceCreateInfo(queueCreateInfos, deviceFeatures, deviceExtensions);

		VkPhysicalDeviceShaderDrawParametersFeatures deviceDrawParametersInfo{};
		deviceDrawParametersInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
		deviceDrawParametersInfo.pNext = nullptr;
		deviceDrawParametersInfo.shaderDrawParameters = VK_TRUE;

		deviceCreateInfo.pNext = &deviceDrawParametersInfo;

		VK_CHECK(vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device));

		_deletionQueue.push_function([=]()
			{
				vkDestroyDevice(_device, nullptr);
			});
	}

	void DeviceHandler::release()
	{
		_deletionQueue.flush();
	}
}