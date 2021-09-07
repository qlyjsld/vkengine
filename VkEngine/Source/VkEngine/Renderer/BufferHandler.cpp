#include "VkEngine/Renderer/BufferHandler.h"
#include "VkEngine/Renderer/DeviceHandler.h"
#include "VkEngine/Core/DeletionQueue.h"
#include "VkEngine/Core/GlobalMacro.h"

#include <vulkan/vulkan.h>

namespace VkEngine
{

	VmaAllocator BufferHandler::_allocator;

	std::vector<AllocatedBuffer> BufferHandler::_allocatedBuffers;
    std::vector<AllocatedImage> BufferHandler::_allocatedImages;

	BufferID BufferHandler::createBuffer(size_t allocSize, VkBufferUsageFlags bufferUsage, VmaMemoryUsage memoryUsage)
	{
		// allocate vertex buffer
		VkBufferCreateInfo bufferinfo{};
		bufferinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferinfo.pNext = nullptr;

		bufferinfo.size = allocSize;
		bufferinfo.usage = bufferUsage;

		VmaAllocationCreateInfo vmaallocInfo{};
		vmaallocInfo.usage = memoryUsage;

		AllocatedBuffer newbuffer;
		newbuffer.bufferSize = allocSize;

		VK_CHECK(vmaCreateBuffer(_allocator, &bufferinfo, &vmaallocInfo, &newbuffer._buffer, &newbuffer._allocation, nullptr));

		_allocatedBuffers.push_back(newbuffer);

		DeletionQueue::push_function([=]()
		{
			vmaDestroyBuffer(_allocator, newbuffer._buffer, newbuffer._allocation);
		});

		return _allocatedBuffers.size() - 1;
	}

    BufferID BufferHandler::createImage(VkFormat format, VkExtent3D extent, VkImageUsageFlags imageUsage, VmaMemoryUsage memoryUsage)
	{
		// the depth image will be an image with the format we selected and Depth Attachment usage flag
        VkImageCreateInfo depthImageCreateInfo{};
        depthImageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        depthImageCreateInfo.pNext = nullptr;

        depthImageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
        depthImageCreateInfo.format = format;
        depthImageCreateInfo.extent = extent;

        depthImageCreateInfo.mipLevels = 1;
        depthImageCreateInfo.arrayLayers = 1;
        depthImageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        depthImageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        depthImageCreateInfo.usage = imageUsage;

        // for the depth image, we want to allocate it from GPU local memory
        VmaAllocationCreateInfo depthImageAllocInfo{};
        depthImageAllocInfo.usage = memoryUsage;
        depthImageAllocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

		AllocatedImage newImage;
        VK_CHECK(vmaCreateImage(_allocator, &depthImageCreateInfo, &depthImageAllocInfo, &newImage._image, &newImage._allocation, nullptr));

		_allocatedImages.push_back(newImage);

		DeletionQueue::push_function([=]()
        {
            vmaDestroyImage(_allocator, newImage._image, newImage._allocation);
        });

		return _allocatedImages.size() - 1;
	}

	void BufferHandler::deleteBuffer(BufferID id)
	{
		vmaDestroyBuffer(_allocator, getBuffer(id)->_buffer, getBuffer(id)->_allocation);
	}

	void BufferHandler::deleteImage(BufferID id)
	{
		vmaDestroyImage(_allocator, getImage(id)->_image, getImage(id)->_allocation);
	}

    AllocatedBuffer* BufferHandler::getBuffer(BufferID bufferId)
	{
		if (bufferId >= 0 && bufferId < _allocatedBuffers.size())
		{
			return &_allocatedBuffers[bufferId];
		}
		else
		{
			throw std::runtime_error("Buffer not exist!");
		}
	}

    AllocatedImage* BufferHandler::getImage(ImageID imageId)
	{
		if (imageId >= 0 && imageId < _allocatedImages.size())
		{
			return &_allocatedImages[imageId];
		}
		else
		{
			throw std::runtime_error("Image not exist!");
		}
	}

	size_t BufferHandler::getBufferSize(BufferID bufferID)
	{
		return _allocatedBuffers[bufferID].bufferSize;
	}

	VmaAllocator BufferHandler::getAllocator()
	{
		return _allocator;
	}

    BufferHandler::BufferHandler(VkInstance instance, DeviceHandler* deviceHandle)
    {
        // initialize the memory allocator
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.physicalDevice = deviceHandle->getPhysicalDevice();
		allocatorInfo.device = deviceHandle->getDevice();
		allocatorInfo.instance = instance;
		vmaCreateAllocator(&allocatorInfo, &_allocator);
	
		DeletionQueue::push_function([&]()
		{
			vmaDestroyAllocator(_allocator);
		});
    }
}