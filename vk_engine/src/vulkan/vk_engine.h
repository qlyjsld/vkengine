#pragma once
#include "vk_support.h"
#include "vk_mesh.h"
#include <deque>
#include <functional>
#include <glm/glm.hpp>

constexpr unsigned int FRAME_OVERLAP = 2;

class PipelineBuilder {
public:

	VkPipelineShaderStageCreateInfo* _shaderStages;
	VkPipelineVertexInputStateCreateInfo _vertexInputInfo;
	VkPipelineInputAssemblyStateCreateInfo _inputAssembly;
	// VkViewport _viewport;
	// VkRect2D _scissor;
	VkPipelineDynamicStateCreateInfo _dynamicState;
	VkPipelineViewportStateCreateInfo _viewportState;
	VkPipelineRasterizationStateCreateInfo _rasterizer;
	VkPipelineColorBlendAttachmentState _colorBlendAttachment;
	VkPipelineMultisampleStateCreateInfo _multisampling;
	VkPipelineLayout _pipelineLayout;
	VkPipelineDepthStencilStateCreateInfo _depthStencil;
	VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
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

struct GPUCameraData {
	glm::mat4 view;
	glm::mat4 projection;
	glm::mat4 viewproj;
};

struct GPUSceneData {
	glm::vec4 fogColor; // w is for exponent
	glm::vec4 fogDistances; // x for min, y for max, zw unused
	glm::vec4 ambientColor;
	glm::vec4 sunlightDirection; // w for sun power
	glm::vec4 sunlightColor;
};

struct FrameData {
	VkCommandPool _commandPool;
	VkCommandBuffer _maincommandBuffer;

	VkSemaphore _imageAvailableSemaphore;
	VkSemaphore _renderFinishedSemaphore;
	VkFence _inFlightFences;

	AllocatedBuffer _objectBuffer;
	VkDescriptorSet _objectDescriptor;
};

struct GPUObjectData {
	glm::mat4 modelMatrix;
};

struct MeshPushConsts {
	glm::vec4 data;
	glm::mat4 render_matrix;
};

struct Material {
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject {
	Mesh* mesh;
	Material* material;
	glm::mat4 transformMatrix;
};

class vk_engine {
public:
	// public functions
	void run();

	// default array of renderable objects
	std::vector<RenderObject> _renderables;

	std::unordered_map<std::string, Material> _materials;
	std::unordered_map<std::string, Mesh> _meshes;

	Material* create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);
	Material* get_material(const std::string& name);

	Mesh* get_mesh(const std::string& name);

	void draw_objects(VkCommandBuffer cmd, RenderObject* first, int count, const FrameData& frame);

private:
	// camera in use
	struct Camera* _camera{ nullptr };

	// handles
	struct GLFWwindow* _window{ nullptr };
	DeletionQueue _deletionQueue;

	// instance and device
	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debugmessager;
	VkSurfaceKHR _surface;
	VkPhysicalDevice _physicalDevice;
	VkDevice _device;

	// queue
	VkQueue _graphicsQueue;
	VkQueue _presentQueue;
	QueueFamilyIndices _indices;

	// swapchain
	VkSwapchainKHR _swapChain;
	std::vector<VkImage> _swapChainImages;
	VkFormat _swapChainImageFormat;
	VkExtent2D _swapChainExtent;
	std::vector<VkImageView> _swapChainImageViews;
	VkImageView _depthImageView;
	AllocatedImage _depthImage;
	VkFormat _depthFormat;

	// renderpass
	VkRenderPass _renderpass;

	// pipeline
	VkPipelineLayout _pipelineLayout;
	VkPipeline _graphicsPipeline;

	// framebuffer
	std::vector<VkFramebuffer> _swapChainFrameBuffers;

	VkPhysicalDeviceProperties _deviceProperties;

	FrameData _frames[FRAME_OVERLAP];

	size_t currentFrame{ 0 };
	size_t _frameNumber{ 0 };
	float lastFrame{ 0 };

	VmaAllocator _allocator; // vma lib allocator

	// callback
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);

	// init functions
	void init_window();
	void init_input();
	void init_vulkan();
	void init_scene();

	// creatre functions
	void createInstance();
	void createLogicalDevice();
	void createSwapChain();
	void createRenderPass();
	void createDescriptors();
	void createGraphicsPipeline();
	VkShaderModule createShaderModule(const std::vector<char>& code);
	void createFrameBuffers();
	void createCommands();
	void createSyncObjects();

	// mainloop functions
	void mainloop();
	void drawFrame();
	void cleanup();

	// Swap Chain related
	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	// meshes
	void load_meshes();
	void upload_mesh(Mesh& mesh);

	// descriptor sets
	VkDescriptorSetLayout _globalSetLayout;
	VkDescriptorSetLayout _objectSetLayout;
	VkDescriptorPool _descriptorPool;

	// buffer for gpu
	AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	// scene parameters
	GPUSceneData _sceneParameters;
	AllocatedBuffer _sceneParametersBuffer;
	GPUCameraData _cameraParameters;
	AllocatedBuffer _cameraParametersBuffer;

	VkDescriptorSet _globalDescriptor;

	size_t pad_uniform_buffer_size(size_t originalSize);
};