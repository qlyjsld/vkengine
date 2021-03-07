#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

// vulkan related error detection macro
#define VK_CHECK(x)\
	do{\
		VkResult err = x;\
		if (err){\
			std::cout << "Error: " << err << std::endl;\
			abort();\
		}\
	} while (0);

#define WIDTH 800
#define HEIGHT 600

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicFamily.has_value() && presentFamily.has_value();
	}
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities{};
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

const std::vector<const char*> validationLayers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static std::vector<char> readfile(const std::string& filename);

class vk_engine {
public:
	// public functions
	void run();

private:
	// handles
	struct GLFWwindow* _window { nullptr };
	VkInstance _instance{};
	VkDebugUtilsMessengerEXT _debugmessager{};
	VkSurfaceKHR _surface{};
	VkPhysicalDevice _physicalDevice{ VK_NULL_HANDLE };
	VkDevice _device{};
	VkQueue _graphicsQueue{};
	VkQueue _presentQueue{};
	VkSwapchainKHR _swapChain{};
	std::vector<VkImage> _swapChainImages{};
	VkFormat _swapChainImageFormat{};
	VkExtent2D _swapChainExtent{};
	std::vector<VkImageView> _swapChainImageViews{};
	VkRenderPass _renderpass{};
	VkPipelineLayout _pipelineLayout{};
	VkPipeline _graphicsPipeline{};

	// callbacks
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	// functions
	void init_window();
	void init_vulkan();
	void createInstance();
	void setupDebugMessenger();
	void createSurface();
	void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void pickPhysicalDevice();
	bool isDeviceSuitable(const VkPhysicalDevice& device);
	QueueFamilyIndices findQueueFamilies(const VkPhysicalDevice& device);
	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device);
	void createLogicalDevice();
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
	void createSwapChain();
	void createImageViews();
	void createRenderPass();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void mainloop();
	void cleanup();

	// Extensions / Layers related functions
	bool checkValidationLayerSupport();
	bool checkDeviceExtensionsSupport(const VkPhysicalDevice& device);
	std::vector<const char*> getRequiredExtension();
	VkResult CreateDebugUtilsMessengerEXT(const VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
	void DestroyDebugUtilsMessengerEXT(const VkInstance& instance, VkDebugUtilsMessengerEXT pDebugMessenger, const VkAllocationCallbacks* pAllocator);
};