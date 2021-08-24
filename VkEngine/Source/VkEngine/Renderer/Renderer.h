#pragma once

#include <vulkan/vulkan.h>

namespace VkEngine
{

	class DeviceHandler;
	class SurfaceHandler;
	class BufferHandler;
	class SwapChainHandler;
	class DescriptorHandler;
	class RenderPassHandler;
	class PipelineHandler;

	struct FrameData
	{
		VkCommandPool _commandPool;
		VkCommandBuffer _maincommandBuffer;

		VkSemaphore _imageAvailableSemaphore;
		VkSemaphore _renderFinishedSemaphore;
		VkFence _inFlightFences;
	};

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
			_pipelineHandle = new PipelineHandler(_deviceHandle, _descriptorHandle, _renderPassHandle);

			initFrameBuffer();
			initCommand();
			initSyncObject();
		}

		~Renderer()
		{
			DeletionQueue::flush();
			release();
		}

	private:

		DeviceHandler* _deviceHandle = nullptr;
		SurfaceHandler* _surfaceHandle = nullptr;
		BufferHandler* _bufferHandle = nullptr;
		SwapChainHandler* _swapChainHandle = nullptr;
		DescriptorHandler* _descriptorHandle = nullptr;
		RenderPassHandler* _renderPassHandle = nullptr;
		PipelineHandler* _pipelineHandle = nullptr;

		VkInstance _instance;
		VkDebugUtilsMessengerEXT _debugMessager;

		std::vector<VkFramebuffer> _swapChainFrameBuffers;

		// frame data
		FrameData _frames[FRAME_OVERLAP];

		void init();
		void release();

		void initFrameBuffer();
		void initCommand();
		void initSYncObject();

		// debug callback
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	};
}