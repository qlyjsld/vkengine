#pragma once

#include <vulkan/vulkan.h>
#include <vector>
#include <functional>

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

	struct UploadContext
	{
		VkFence _uploadFence;
		VkCommandPool _commandPool;
	};

	class Renderer
	{
	public:

		Renderer();
		~Renderer();

		// immediate commands
		UploadContext _uploadContext;
		void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& func);

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
		FrameData* _frames;

		int FRAME_OVERLAP;

		void init();
		void release();

		void initFrameBuffer();
		void initCommand();
		void initSyncObject();

		// debug callback
		static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
	};
}