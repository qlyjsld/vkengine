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

struct GPUObjectData {
	glm::mat4 modelMatrix;
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

struct Material {
	VkDescriptorSet textureSet{ VK_NULL_HANDLE };

	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;
};

struct RenderObject {
	Mesh* mesh;
	Material* material;
	glm::mat4 transformMatrix;
};

struct Texture {
	AllocatedImage Image;
	VkImageView imageView;
};

struct UploadContext {
	VkFence _uploadFence;
	VkCommandPool _commandPool;
};

// vk_engine is a Vulkan rendering engine
class vk_engine {
public:
	// default camera
	struct Camera* _camera{ nullptr };

	// default window
	struct GLFWwindow* _window{ nullptr };

	// default entry point
	void run();

	// queue for destruction of Vulkan objects
	DeletionQueue _deletionQueue;

	// default array of renderable objects
	std::vector<RenderObject> _renderables;

	/* _material stores pipeline of meshes
	* _meshes stores vertices of meshes
	* _textures store textures of meshes
	*/
	std::unordered_map<std::string, Material> _materials;
	std::unordered_map<std::string, Mesh> _meshes;
	std::unordered_map<std::string, Texture> _textures;

	void load_meshes(); // load meshes data into _meshes
	void upload_mesh(Mesh& mesh); // upload meshes data to gpu
	void load_textures(); // load textures into _textures

	// load / store from the unordered maps
	Mesh* get_mesh(const std::string& name);
	Material* get_material(const std::string& name);
	Material* create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name);

	// draw functions
	void drawFrame();
	void draw_objects(VkCommandBuffer cmd, RenderObject* first, int count, const FrameData& frame);

	// Vulkan memory allocator
	VmaAllocator _allocator;

	// create buffer for gpu
	AllocatedBuffer create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage);

	// immediate commands
	UploadContext _uploadContext;
	void immediate_submit(std::function<void(VkCommandBuffer cmd)>&& func);

	// frame data
	FrameData _frames[FRAME_OVERLAP];

	size_t _currentFrame{ 0 };
	size_t _frameNumber{ 0 };
	float _lastFrame{ 0 };

private:
	// instance and device handler
	VkInstance _instance;
	VkDebugUtilsMessengerEXT _debugmessager;
	VkSurfaceKHR _surface;
	VkPhysicalDevice _physicalDevice;
	VkDevice _device;

	// gpu queue for command submission handler
	VkQueue _graphicsQueue;
	VkQueue _presentQueue;
	QueueFamilyIndices _indices;

	// swapchain handler
	VkSwapchainKHR _swapChain;
	std::vector<VkImage> _swapChainImages;
	VkFormat _swapChainImageFormat;
	VkExtent2D _swapChainExtent;
	std::vector<VkImageView> _swapChainImageViews;

	// depth image handler
	VkImageView _depthImageView;
	AllocatedImage _depthImage;
	VkFormat _depthFormat;

	// renderpass handler
	VkRenderPass _renderpass;

	// pipeline handler
	VkPipelineLayout _pipelineLayout;
	VkPipeline _graphicsPipeline;

	// framebuffer handler
	std::vector<VkFramebuffer> _swapChainFrameBuffers;

	// device properties
	VkPhysicalDeviceProperties _deviceProperties;

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

	// main functions
	void mainloop();
	void cleanup();

	// Swap Chain related
	SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice& device);
	VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	// descriptor sets
	VkDescriptorSet _globalDescriptor;

	VkDescriptorSetLayout _globalSetLayout;
	VkDescriptorSetLayout _objectSetLayout;
	VkDescriptorSetLayout _textureSetLayout;
	VkDescriptorPool _descriptorPool;

	// scene parameters
	GPUSceneData _sceneParameters;
	AllocatedBuffer _sceneParametersBuffer;
	GPUCameraData _cameraParameters;
	AllocatedBuffer _cameraParametersBuffer;

	// calculation of uniform buffer size
	size_t pad_uniform_buffer_size(size_t originalSize);

	// debug callback
	static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
};