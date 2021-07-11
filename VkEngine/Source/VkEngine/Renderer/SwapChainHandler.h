#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

#include "VkEngine/Renderer/DeviceHandler.h"
#include "VkEngine/Renderer/SurfaceHandler.h"
#include "VkEngine/Renderer/BufferHandler.h"

namespace VkEngine
{

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

    class SwapChainHandler
    {
    public:

        SwapChainHandler(DeviceHandler* deviceHandle, SurfaceHandler* surfaceHandle)
        {
            init(deviceHandle, surfaceHandle);
        }

        ~SwapChainHandler()
        {
            release();
        }

        VkSwapchainKHR _swapChain;
        VkFormat _swapChainImageFormat;
		VkExtent2D _swapChainExtent;

        VkQueue _graphicsQueue;
		VkQueue _presentQueue;

        std::optional<uint32_t> _graphicFamilyIndex;
		std::optional<uint32_t> _presentFamilyIndex;

		std::vector<VkImage> _swapChainImages;
		std::vector<VkImageView> _swapChainImageViews;

        ImageID _depthImage;
        VkImageView _depthImageView;

        QueueFamilyIndices _indices;

        void init(DeviceHandler* deviceHandle, SurfaceHandler* surfaceHandle);
        void release();

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> availablePresentModes);
		VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities);
    };
}