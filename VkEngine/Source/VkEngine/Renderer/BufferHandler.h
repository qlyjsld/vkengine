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
        size_t bufferSize;
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

        BufferHandler(VkInstance instance, DeviceHandler* deviceHandle);
        ~BufferHandler() {};

        static BufferID createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);
        static BufferID createImage(VkFormat format, VkExtent3D extent, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage);

        static inline AllocatedBuffer& getBuffer(BufferID bufferId);
        static inline AllocatedImage& getImage(ImageID imageId);

        static inline size_t getBufferSize(BufferID bufferID);
        
    private:

        static VmaAllocator _allocator;

        static std::vector<AllocatedBuffer> _allocatedBuffers;
        static std::vector<AllocatedImage> _allocatedImages;
    };
}
