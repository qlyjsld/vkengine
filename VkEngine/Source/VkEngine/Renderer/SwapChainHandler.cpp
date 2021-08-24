#include "VkEngine/Renderer/SwapChainHandler.h"
#include "VkEngine/Renderer/DeviceHandler.h"
#include "VkEngine/Renderer/SurfaceHandler.h"
#include "VkEngine/Renderer/BufferHandler.h"
#include "VkEngine/Renderer/DeletionQueue.h"

constexpr VkPresentModeKHR PRESENTMODE = VK_PRESENT_MODE_IMMEDIATE_KHR; // VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR

namespace VkEngine
{

    void SwapChainHandler::SwapChainHandler(DeviceHandler* deviceHandle, SurfaceHandler* surfaceHandle)
    {
        // get queue
        vkGetDeviceQueue(deviceHandle->getDevice(), _indices.graphicFamily.value(), 0, &_graphicsQueue);
        vkGetDeviceQueue(deviceHandle->getDevice(), _indices.presentFamily.value(), 0, &_presentQueue);

        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(deviceHandle->getPhysicalDevice());
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D _swapChainExtent = chooseSwapExtent(swapChainSupport.capabilities);

        _swapChainImageFormat = surfaceFormat.format;

        uint32_t image_count = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.maxImageCount > 0 && image_count > swapChainSupport.capabilities.maxImageCount)
        {
            image_count = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR swapChainCreateInfo{};
        swapChainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swapChainCreateInfo.surface = surfaceHandle->getSurface();

        swapChainCreateInfo.minImageCount = image_count;
        swapChainCreateInfo.imageFormat = surfaceFormat.format;
        swapChainCreateInfo.imageColorSpace = surfaceFormat.colorSpace;
        swapChainCreateInfo.imageExtent = _swapChainExtent;
        swapChainCreateInfo.imageArrayLayers = 1;
        swapChainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        uint32_t queueFamilyIndices[] = { _indices.graphicFamily.value(), _indices.presentFamily.value() };

        if (_indices.graphicFamily != _indices.presentFamily) {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swapChainCreateInfo.queueFamilyIndexCount = 2;
            swapChainCreateInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            swapChainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swapChainCreateInfo.queueFamilyIndexCount = 0;
            swapChainCreateInfo.pQueueFamilyIndices = nullptr;
        }

        swapChainCreateInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        swapChainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swapChainCreateInfo.presentMode = presentMode;
        swapChainCreateInfo.clipped = VK_TRUE;
        swapChainCreateInfo.oldSwapchain = VK_NULL_HANDLE;

        VK_CHECK(vkCreateSwapchainKHR(deviceHandle->getDevice(), &swapChainCreateInfo, nullptr, &_swapChain));

        DeletionQueue::push_function([=]()
        {
            vkDestroySwapchainKHR(deviceHandle->getDevice(), _swapChain, nullptr);
        });

        vkGetSwapchainImagesKHR(deviceHandle->getDevice(), _swapChain, &image_count, nullptr);
        _swapChainImages.resize(image_count);
        vkGetSwapchainImagesKHR(deviceHandle->getDevice(), _swapChain, &image_count, _swapChainImages.data());

        // create Image Views
        _swapChainImageViews.resize(_swapChainImages.size());

        for (size_t i = 0; i < _swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo imageViewCreateInfo{};
            imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            imageViewCreateInfo.pNext = nullptr;
            imageViewCreateInfo.image = _swapChainImages[i];

            imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            imageViewCreateInfo.format = _swapChainImageFormat;

            imageViewCreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            imageViewCreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
            imageViewCreateInfo.subresourceRange.levelCount = 1;
            imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
            imageViewCreateInfo.subresourceRange.layerCount = 1;

            VK_CHECK(vkCreateImageView(deviceHandle->getDevice(), &imageViewCreateInfo, nullptr, &_swapChainImageViews[i]));

            DeletionQueue::push_function([=](){
                vkDestroyImageView(deviceHandle->getDevice(), _swapChainImageViews[i], nullptr);
            });
        }

        VkExtent3D depthImageExtent =
        {
            _swapChainExtent.width,
            _swapChainExtent.height,
            1
        };

        _depthImageFormat = VK_FORMAT_D32_SFLOAT;

        // create the depth image
        _depthImage = BufferHandler::createImage(VK_FORMAT_D32_SFLOAT, depthImageExtent, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VMA_MEMORY_USAGE_GPU_ONLY);

        DeletionQueue::push_function([=]()
        {
            vmaDestroyImage(_allocator, _depthImage._image, _depthImage._allocation);
        });

        // build an image-view for the depth image to use for rendering
        VkImageViewCreateInfo depthImageViewcreateInfo{};
        depthImageViewcreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        depthImageViewcreateInfo.pNext = nullptr;
        depthImageViewcreateInfo.image = BufferHandler::getImage(_depthimage);

        depthImageViewcreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        depthImageViewcreateInfo.format = VK_FORMAT_D32_SFLOAT;

        depthImageViewcreateInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthImageViewcreateInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthImageViewcreateInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        depthImageViewcreateInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

        depthImageViewcreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        depthImageViewcreateInfo.subresourceRange.baseMipLevel = 0;
        depthImageViewcreateInfo.subresourceRange.levelCount = 1;
        depthImageViewcreateInfo.subresourceRange.baseArrayLayer = 0;
        depthImageViewcreateInfo.subresourceRange.layerCount = 1;

        VK_CHECK(vkCreateImageView(_device, &depthImageViewcreateInfo, nullptr, &_depthImageView));

        DeletionQueue::push_function([=]()
        {
            vkDestroyImageView(_device, _depthImageView, nullptr);
        });
    }

    SwapChainSupportDetails SwapChainHandler::querySwapChainSupport(VkPhysicalDevice device)
    {
        SwapChainSupportDetails details;

        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

        if (formatCount != 0)
        {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

        if (presentModeCount != 0)
        {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
        }

        return details;
    }

    VkSurfaceFormatKHR SwapChainHandler::chooseSwapSurfaceFormat(std::vector<VkSurfaceFormatKHR> availableFormats)
    {
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR SwapChainHandler::chooseSwapPresentMode(std::vector<VkPresentModeKHR> availablePresentModes)
    {
        for (const auto& availablePresentMode : availablePresentModes)
        {
            if (availablePresentMode == PRESENTMODE)
            {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapChainHandler::chooseSwapExtent(VkSurfaceCapabilitiesKHR capabilities)
    {
        if (capabilities.currentExtent.width != UINT32_MAX)
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width, height;
            glfwGetFramebufferSize(_window, &width, &height);

            VkExtent2D actualExtent =
            {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }
}
