#pragma once

namespace VkEngine
{

    class DescriptorHandler
    {
    public:

        DescriptorHandler()
        {
            init();
        }

        ~DescriptorHandler();

        void init();
        void release();
    };
}