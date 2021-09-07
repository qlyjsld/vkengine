#pragma once
#include <vulkan/vulkan.h>

namespace VkEngine
{

    class DeviceHandler;
    class SwapChainHandler;

    class RenderPassHandler
    {
    public:

        RenderPassHandler(SwapChainHandler* swapChainHandler, DeviceHandler* deviceHandler);

        ~RenderPassHandler() {};

        inline VkRenderPass getRenderpass() { return _renderpass; };

    private:

        VkRenderPass _renderpass;
    };
}