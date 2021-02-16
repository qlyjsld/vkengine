#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <optional>

const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

#ifdef NDEBUG
	const bool enableValidationLayers = false;
#else
	const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
	std::optional<uint32_t> graphicFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete() {
		return graphicFamily.has_value() && presentFamily.has_value();
	}
};

// Extensions / Layers related functions
bool checkValidationLayerSupport();
std::vector<const char*> getRequiredExtension();
VkResult CreateDebugUtilsMessengerEXT(const VkInstance& instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);
void DestroyDebugUtilsMessengerEXT(const VkInstance& instance, VkDebugUtilsMessengerEXT pDebugMessenger, const VkAllocationCallbacks* pAllocator);

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
	void createLogicalDevice();
	void mainloop();
	void cleanup();
};