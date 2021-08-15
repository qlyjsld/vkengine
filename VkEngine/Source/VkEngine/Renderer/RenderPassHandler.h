#pragma once
#include <vulkan/vulkan.h>

#include "VkEngine/Renderer/SwapChainHandler.h"

namespace VkEngine
{

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