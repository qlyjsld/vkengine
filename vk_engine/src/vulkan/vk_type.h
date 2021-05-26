#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

#include "vk_mem_alloc.h"

struct AllocatedBuffer {
	VkBuffer _buffer;
	VmaAllocation _allocation;
};

struct AllocatedImage {
	VkImage _image;
	VmaAllocation _allocation;
};

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
};
