#include "VkEngine/Renderer/Renderer.h"
#include "VkEngine/Renderer/DeviceHandler.h"
#include "VkEngine/Renderer/SurfaceHandler.h"
#include "VkEngine/Renderer/BufferHandler.h"
#include "VkEngine/Renderer/SwapChainHandler.h"
#include "VkEngine/Renderer/DescriptorHandler.h"
#include "VkEngine/Renderer/RenderPassHandler.h"
#include "VkEngine/Renderer/PipelineHandler.h"
#include "VkEngine/Renderer/DeletionQueue.h"
#include "VkEngine/Core/ConsoleVariableSystem.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <stdexcept>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

// vulkan related error detection macro
#define VK_CHECK(x)\
	do\
	{\
		VkResult err = x;\
		if (err){\
			std::cout << "Error: " << err << std::endl;\
			abort();\
		}\
	} while (0);

const std::vector<const char*> validationLayers =
{
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_SHADER_DRAW_PARAMETERS_EXTENSION_NAME
};

namespace VkEngine
{

	void Renderer::init()
	{
		// check validation layers support
		uint32_t layerCount;
		vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

		std::vector<VkLayerProperties> availableLayers(layerCount);
		vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

		for (const char* layerName : validationLayers)
		{
			bool layer_found = false;

			for (const auto& layerProperties : availableLayers)
			{
				if (strcmp(layerName, layerProperties.layerName) == 0)
				{
					layer_found = true;
					break;
				}
			}
			if (enableValidationLayers && !layer_found)
			{
				throw std::runtime_error("validation layers requested, but not available!");
			}
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.apiVersion = VK_API_VERSION_1_2;

		// instance info
		VkInstanceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// get extensions
		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

		if (enableValidationLayers)
		{
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}

		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
		debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		debugCreateInfo.pfnUserCallback = debugCallback;
		debugCreateInfo.pUserData = nullptr;

		// enable validation layers
		if (enableValidationLayers)
		{
			createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
			createInfo.ppEnabledLayerNames = validationLayers.data();
			createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
		}
		else
		{
			createInfo.enabledLayerCount = 0;
		}

		// create the _instance
		VK_CHECK(vkCreateInstance(&createInfo, nullptr, &_instance));

		DeletionQueue::push_function([=]()
		{
			vkDestroyInstance(_instance, nullptr);
		});

		// setup debug messenger
		if (enableValidationLayers)
		{
			auto createDebugUtilsMessengerfunc = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkCreateDebugUtilsMessengerEXT");
			if (createDebugUtilsMessengerfunc != nullptr)
			{
				createDebugUtilsMessengerfunc(_instance, &debugCreateInfo, nullptr, &_debugMessager);
			}
			else
			{
				throw std::runtime_error("failed to set up debug messenger!");
			}

			auto destroyDebugUtilsMessengerfunc = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(_instance, "vkDestroyDebugUtilsMessengerEXT");

			DeletionQueue::push_function([=]()
			{
				destroyDebugUtilsMessengerfunc(_instance, _debugMessager, nullptr);
			});
		}
	}

	void Renderer::initFrameBuffer()
	{
		_swapChainFrameBuffers.resize(ConsoleVariableSystem::get()->getIntVariableCurrentByHash("FRAME_OVERLAP"));

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = _renderPassHandle->getRenderpass();
		framebufferInfo.attachmentCount = 1;
		framebufferInfo.width = _swapChainHandle->getSwapChainExtent().width;
		framebufferInfo.height = _swapChainHandle->getSwapChainExtent().height;
		framebufferInfo.layers = 1;

		for (size_t i = 0; i < _swapChainFrameBuffers.size(); i++)
		{
			VkImageView attachments[] =
			{
				_swapChainHandle->getSwapChainImageViews()[i],
				_depthImageView
			};
			framebufferInfo.pAttachments = attachments;
			framebufferInfo.attachmentCount = 2;

			VK_CHECK(vkCreateFramebuffer(_device, &framebufferInfo, nullptr, _swapChainHandle->getSwapChainImageViews()[i]));

			DeletionQueue::push_function([=]()
			{
				vkDestroyFramebuffer(_device, _swapChainFrameBuffers[i], nullptr);
			});
		}
	}

	void Renderer::initCommand()
	{
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = _indices.graphicFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // optional

		for (auto& frame : _frames)
		{
			VK_CHECK(vkCreateCommandPool(_device, &poolInfo, nullptr, &frame._commandPool));

			DeletionQueue::push_function([=]()
			{
				vkDestroyCommandPool(_device, frame._commandPool, nullptr);
			});

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = frame._commandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			VK_CHECK(vkAllocateCommandBuffers(_device, &allocInfo, &frame._maincommandBuffer));
		}

		VK_CHECK(vkCreateCommandPool(_device, &poolInfo, nullptr, &_uploadContext._commandPool));

		DeletionQueue::push_function([=]()
		{
			vkDestroyCommandPool(_device, _uploadContext._commandPool, nullptr);
		});
	}

	void Renderer::initSYncObject()
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (auto& frame : _frames)
		{
			VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &frame._imageAvailableSemaphore));
			VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &frame._renderFinishedSemaphore));
			VK_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &frame._inFlightFences));
			DeletionQueue::push_function([=]()
			{
				vkDestroySemaphore(_device, frame._imageAvailableSemaphore, nullptr);
				vkDestroySemaphore(_device, frame._renderFinishedSemaphore, nullptr);
				vkDestroyFence(_device, frame._inFlightFences, nullptr);
			});
		}

		fenceInfo.flags = NULL;

		VK_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &_uploadContext._uploadFence));

		DeletionQueue::push_function([=]()
		{
			vkDestroyFence(_device, _uploadContext._uploadFence, nullptr);
		});
	}

	void Renderer::release()
	{
		DeletionQueue::flush();

		if (_device != nullptr)
		{
			delete _device;
		}

		if (_present != nullptr)
		{
			delete _present;
		}

		if (_pipeline != nullptr)
		{
			delete _pipeline;
		}
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL Renderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
	{
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}
}