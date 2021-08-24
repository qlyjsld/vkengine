#pragma once
#include <vulkan/vulkan.h>

namespace VkEngine
{

    class SwapChainHandler;

    class RenderPassHandler
    {
    public:

        RenderPassHandler(SwapChainHandler* swapChainHandler);

        ~RenderPassHandler() {};

        inline getRenderpass() { return _renderpass; };

    private:

        VkRenderpass _renderpass;
    };
}