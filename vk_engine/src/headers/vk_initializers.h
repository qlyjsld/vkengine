#pragma once

#include <vulkan/vulkan.h>

namespace vk_init {
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void pickPhysicalDevice();
	void createLogicalDevice();
}