#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

struct GLFWwindow;

namespace VkEngine
{

    class DeviceHandler;
    class SurfaceHandler;
    class BufferHandler;

	struct SwapChainSupportDetails
	{
		VkSurfaceCapabilitiesKHR capabilities;
		std::vector<VkSurfaceFormatKHR> formats;
		std::vector<VkPresentModeKHR> presentModes;
	};

    class SwapChainHandler
    {
    public:

        SwapChainHandler(DeviceHandler* deviceHandle, SurfaceHandler* surfaceHandle);
        ~SwapChainHandler() {};

        inline VkSwapchainKHR getSwapChain() { return _swapChain; };
        inline std::vector<VkImageView>& getSwapChainImageViews() { return _swapChainImageViews; };
        inline VkFormat getSwapChainImageFormat() { return _swapChainImageFormat; };
        inline VkExtent2D getSwapChainExtent() { return _swapChainExtent; };
        inline VkImageView getDepthImageView() { return _depthImageView; };
        inline VkFormat getDepthImageFormat() { return _depthImageFormat; };

        inline uint32_t getGraphicFamilyIndex() { return _graphicFamilyIndex.has_value() ? _graphicFamilyIndex.value() : -1; };
        inline uint32_t getPresentFamilyIndex() { return _presentFamilyIndex.has_value() ? _presentFamilyIndex.value() : -1; };

        inline VkQueue getGraphicQueue() { return _graphicsQueue; };
        inline VkQueue getPresentQueue() { return _presentQueue; };

    private:

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
        VkFormat _depthImageFormat;

        QueueFamilyIndices _indices;

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface);
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(std::vector<VkPresentModeKHR> availablePresentModes);
		VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities, GLFWwindow* window);
    };
}