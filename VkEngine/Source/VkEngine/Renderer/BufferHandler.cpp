#include "VkEngine/Renderer/BufferHandler.h"
#include "VkEngine/Renderer/DeletionQueue.h"

namespace VkEngine
{

	BufferID BufferHandler::createBuffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage)
	{
		// allocate vertex buffer
		VkBufferCreateInfo bufferinfo{};
		bufferinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferinfo.pNext = nullptr;

		bufferinfo.size = allocSize;
		bufferinfo.usage = usage;

		VmaAllocationCreateInfo vmaallocInfo{};
		vmaallocInfo.usage = memoryUsage;

		AllocatedBuffer newbuffer;
		VK_CHECK(vmaCreateBuffer(_allocator, &bufferinfo, &vmaallocInfo, &newbuffer._buffer, &newbuffer._allocation, nullptr));

		_allocatedBuffers.push_back(newbuffer);

		return _allocatedBuffers.size() - 1;
	}

    BufferID BufferHandler::createImage()
	{

	}

    AllocatedBuffer& BufferHandler::getBuffer(BufferID bufferId)
	{
		if (bufferId >= 0 && bufferId < _allocatedBuffers.size())
		{
			return &_allocatedBuffers[bufferId];
		}
		else
		{
			return std::runtime_error("Buffer not exist!");
		}
	}

    AllocatedImage& BufferHandler::getImage(ImageID imageId)
	{
		if (imageId >= 0 && imageId < _allocatedImages.size())
		{
			return &_allocatedImages[imageId];
		}
		else
		{
			return std::runtime_error("Image not exist!");
		}
	}

    void BufferHandler::init(VkInstance instance, DeviceHandler* deviceHandle)
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

    void BufferHandler::release()
    {
		// do nothing
    }
}