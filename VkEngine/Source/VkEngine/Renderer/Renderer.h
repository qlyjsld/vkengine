#pragma once
#include <vulkan/vulkan.h>
#include "VkEngine/Renderer/DeviceHandler.h"
#include "VkEngine/Renderer/PresentHandler.h"
#include "VkEngine/Renderer/PipelineHandler.h"
#include "VkEngine/Renderer/DeletionQueue.h"

namespace VkEngine
{

	// vk_engine is a Vulkan rendering engine
	class Renderer
	{
	public:

		Renderer()
		{
			init();

			_device = new DeviceHandler(_instance);
			_present = new PresentHandler();
			_pipeline = new PipelineHandler();
		}

		~Renderer()
		{
			release();
		}

		SurfaceHandler* _surface;

		// device handler handles vkdevice
		DeviceHandler* _device;

		// present handler handles swapchain, framebuffers, queueFamiles
		PresentHandler* _present;

		// pipeline handler handles pipeline, shaders, buffers
		PipelineHandler* _pipeline;

		DeletionQueue _deletionQueue;

		VkInstance _instance;
		VkDebugUtilsMessengerEXT _debugMessager;

		void init();
		void release();

		// debug callback
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	};

}