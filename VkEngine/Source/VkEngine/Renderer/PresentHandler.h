#pragma once

#include "VkEngine/Renderer/DeviceHandler.h"
#include "VkEngine/Renderer/SurfaceHandler.h"
#include "VkEngine/Renderer/SwapChainHandler.h"
#include "VkEngine/Renderer/DescriptorHandler.h"
#include "VkEngine/Renderer/RenderPassHandler.h"

namespace VkEngine
{
	
	class PresentHandler
	{
	public:

		PresentHandler(DeviceHandler* deviceHandle, SurfaceHandler* surfaceHandle)
		{
			init();

			_swapChainHandle = new SwapChainHandler(deviceHandle, surfaceHandle);
			_descriptorHandle = new DescriptorHandler(deviceHandle);
			_renderPassHandle = new RenderPassHandler();
		}

		~PresentHandler()
		{
			release();
		};

		SwapChainHandler* _swapChainHandle;
		DescriptorHandler* _descriptorHandle;
		RenderPassHandler* _renderPassHandle;

		void init();
		void release();
	};
}