#pragma once

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "VkEngine/Renderer/DeletionQueue.h"

namespace VkEngine
{

    class BufferHandler
    {
    public:

        BufferHandler()
        {
            init();
        }

        ~BufferHandler()
        {
            release();
        }

        VmaAllocator _allocator;

        DeletionQueue _deletionQueue;

        void init();
        void release();
    };
}