#pragma once
#include "vk_types.h"

// Supports / Extensions related
namespace vk_support {
	std::vector<const char*> getRequiredExtension(const bool& enableValidationLayers);

	bool checkValidationLayerSupport();

	bool checkDeviceExtensionsSupport(const VkPhysicalDevice& device);

	VkResult CreateDebugUtilsMessengerEXT(const VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

	void DestroyDebugUtilsMessengerEXT(const VkInstance& instance, VkDebugUtilsMessengerEXT pDebugMessenger, const VkAllocationCallbacks* pAllocator);

	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device, const VkSurfaceKHR& surface);
}