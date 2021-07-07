#pragma once

#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

#include "VkEngine/Renderer/DeviceHandler.h"

namespace VkEngine
{
    typedef BufferID size_t;
    typedef ImageID size_t;

    struct AllocatedBuffer
    {
        VkBuffer _buffer;
		VmaAllocation _allocation;
    };

    struct AllocatedImage
    {
        VkImage _image;
		VmaAllocation _allocation;
    };

    class BufferHandler
    {
    public:

        BufferHandler(VkInstance instance, DeviceHandler* deviceHandle)
        {
            init(instance, deviceHandle);
        }

        ~BufferHandler()
        {
            release();
        }

        VmaAllocator _allocator;

        std::vector<AllocatedBuffer> _allocatedBuffers;
        std::vector<AllocatedImage> _allocatedImages;

        BufferID createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        BufferID createImage();

        AllocatedBuffer& getBuffer(BufferID bufferId);
        AllocatedImage& getImage(ImageID imageId);

        void init(VkInstance instance, DeviceHandler* deviceHandle);
        void release();
    };
}