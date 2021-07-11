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

        static VmaAllocator _allocator;

        static std::vector<AllocatedBuffer> _allocatedBuffers;
        static std::vector<AllocatedImage> _allocatedImages;

        static BufferID createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        static BufferID createImage(VkFormat format, VkExtent3D extent, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage);

        static AllocatedBuffer& getBuffer(BufferID bufferId);
        static AllocatedImage& getImage(ImageID imageId);

        static void init(VkInstance instance, DeviceHandler* deviceHandle);
        static void release();
    };
}
