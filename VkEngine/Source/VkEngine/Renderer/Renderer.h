#pragma once

#include <vulkan/vulkan.h>

#include "VkEngine/Renderer/DeviceHandler.h"
#include "VkEngine/Renderer/SurfaceHandler.h"
#include "VkEngine/Renderer/BufferHandler.h"
#include "VkEngine/Renderer/SwapChainHandler.h"
#include "VkEngine/Renderer/DescriptorHandler.h"
#include "VkEngine/Renderer/RenderPassHandler.h"
#include "VkEngine/Renderer/PipelineHandler.h"

namespace VkEngine
{

	class Renderer
	{
	public:

		Renderer()
		{
			init();

			_deviceHandle = new DeviceHandler(_instance);
			_surfaceHandle = new SurfaceHandler(_instance);
			_bufferHandle = new BufferHandler(_instance, _deviceHandle);
			_swapChainHandle = new SwapChainHandler(_deviceHandle, _surfaceHandle);
			_descriptorHandle = new DescriptorHandler(_deviceHandle);
			_renderPassHandle = new RenderPassHandler(_swapChainHandle);
			_pipelineHandle = new PipelineHandler(_descriptorHandle);
		}

		~Renderer()
		{
			release();
		}

	private:

		DeviceHandler* _deviceHandle;
		SurfaceHandler* _surfaceHandle;
		BufferHandler* _bufferHandle;
		SwapChainHandler* _swapChainHandle;
		DescriptorHandler* _descriptorHandle;
		RenderPassHandler* _renderPassHandle;
		PipelineHandler* _pipelineHandle;

		VkInstance _instance;
		VkDebugUtilsMessengerEXT _debugMessager;

		void init();
		void release();

		// debug callback
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	};
}