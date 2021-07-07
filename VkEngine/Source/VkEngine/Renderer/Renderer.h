#pragma once

#include <vulkan/vulkan.h>

#include "VkEngine/Renderer/DeviceHandler.h"
#include "VkEngine/Renderer/SurfaceHandler.h"
#include "VkEngine/Renderer/BufferHandler.h"
#include "VkEngine/Renderer/PresentHandler.h"
#include "VkEngine/Renderer/PipelineHandler.h"

namespace VkEngine
{

	// vk_engine is a Vulkan rendering engine
	class Renderer
	{
	public:

		Renderer()
		{
			init();

			_deviceHandle = new DeviceHandler(_instance);
			_surfaceHandle = new SurfaceHandler(_instance);
			_bufferHandle = new BufferHandler(_instance, _deviceHandle);
			_presentHandle = new PresentHandler(_deviceHandle, _surfaceHandle);
			_pipelineHandle = new PipelineHandler();
		}

		~Renderer()
		{
			release();
		}

		DeviceHandler* _deviceHandle;
		SurfaceHandler* _surfaceHandle;
		BufferHandler* _bufferHandle;
		PresentHandler* _presentHandle;
		PipelineHandler* _pipelineHandle;

		VkInstance _instance;
		VkDebugUtilsMessengerEXT _debugMessager;

		void init();
		void release();

		// debug callback
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	};
}