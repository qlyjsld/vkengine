#include "VkEngine/Renderer/BufferHandler.h"

namespace VkEngine
{

    void BufferHandler::init()
    {
        // initialize the memory allocator
		VmaAllocatorCreateInfo allocatorInfo{};
		allocatorInfo.physicalDevice = _physicalDevice;
		allocatorInfo.device = _device;
		allocatorInfo.instance = _instance;
		vmaCreateAllocator(&allocatorInfo, &_allocator);
	
		_deletionQueue.push_function([&]()
		{
			vmaDestroyAllocator(_allocator);
		});
    }

    void BufferHandler::release()
    {
        _deletionQueue.flush();
    }
}