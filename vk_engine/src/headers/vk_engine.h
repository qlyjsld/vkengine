#pragma once
#include "vk_support.h"
#include "vk_mesh.h"
#include <deque>
#include <functional>
#include <glm/glm.hpp>

struct MeshPushConsts {
	glm::vec4 data;
	glm::mat4 render_matrix;
};

struct DeletionQueue {
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& func){
		deletors.push_back(func);
	}

	void flush(){
		for (auto it = deletors.end() - 1; it > deletors.begin(); it--){
			(*it)(); // call function
		}
		deletors.clear();
	}
};

class vk_engine {
public:
	// public functions
	void run();

private:
	// handles
	struct GLFWwindow* _window{ nullptr };
	DeletionQueue _deletionQueue;

	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debugmessager;
	VkSurfaceKHR _surface;
	VkPhysicalDevice _physicalDevice;
	VkDevice _device;

	VkQueue _graphicsQueue;
	VkQueue _presentQueue;
	QueueFamilyIndices _indices;

	VkSwapchainKHR _swapChain;
	std::vector<VkImage> _swapChainImages;
	VkFormat _swapChainImageFormat;
	VkExtent2D _swapChainExtent;
	std::vector<VkImageView> _swapChainImageViews;
	VkImageView _depthImageView;
	AllocatedImage _depthImage;
	VkFormat _depthFormat;

	VkRenderPass _renderpass;

	VkPipelineLayout _pipelineLayout;
	VkPipeline _graphicsPipeline;

	std::vector<VkFramebuffer> _swapChainFrameBuffers;

	VkCommandPool _commandPool;
	std::vector<VkCommandBuffer> _commandBuffers;

	std::vector<VkSemaphore> _imageAvailableSemaphore;
	std::vector<VkSemaphore> _renderFinishedSemaphore;
	std::vector<VkFence> _inFlightFences;
	std::vector<VkFence> _imagesInFlight;

	size_t currentFrame{ 0 };
	size_t _frameNumber{ 0 };

	VmaAllocator _allocator; // vma lib allocator

	Mesh _triangleMesh;
	Mesh _monkeyMesh;

	// callback
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	// functions
	void init_window();
	void init_vulkan();
	void createInstance();
	void createLogicalDevice();
	void createSwapChain();
	void createRenderPass();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createFrameBuffers();
	void createCommands();
	void createSyncObjects();
	void mainloop();
	void drawFrame();
	void cleanup();

	// Swap Chain related
	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	// Meshes
	void load_meshes();

	void upload_mesh(Mesh& mesh);
};