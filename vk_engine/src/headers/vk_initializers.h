#pragma once
#include <vulkan/vulkan.h>

namespace vk_init {
	VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);

	VkDeviceQueueCreateInfo DeviceQueueCreateInfo(const uint32_t& queueFamily, const float& queuePriority);

	VkDeviceCreateInfo DeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, const VkPhysicalDeviceFeatures& deviceFeatures, const std::vector<const char*>& deviceExtensions);
}