#pragma once
#include "vk_support.h"
#include "vk_mesh.h"

class vk_engine {
public:
	// public functions
	void run();

private:
	// handles
	struct GLFWwindow* _window{ nullptr };

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

	VmaAllocator _allocator; // vma lib allocator

	Mesh _triangleMesh;

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