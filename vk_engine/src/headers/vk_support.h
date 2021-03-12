#pragma once
#include "vk_types.h"

// supports or extensions related
namespace vk_support {
	bool checkValidationLayerSupport();

	bool checkDeviceExtensionsSupport(const VkPhysicalDevice& device);

	VkResult CreateDebugUtilsMessengerEXT(const VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

	void DestroyDebugUtilsMessengerEXT(const VkInstance& instance, VkDebugUtilsMessengerEXT pDebugMessenger, const VkAllocationCallbacks* pAllocator);
}