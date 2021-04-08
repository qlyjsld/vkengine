#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <set>

#define VMA_IMPLEMENTATION
#include "vk_engine.h"
#include "vk_initializers.h"
#include "glm/gtc/matrix_transform.hpp"

#include "Camera.h"

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

const float WIDTH = 1600.0f;
const float HEIGHT = 900.0f;

#define MAX_FRAMES_IN_FLIGHT 2

const VkClearValue clearColor = { 0.25f, 0.25f, 0.25f, 1.0f };

static std::vector<char> readfile(const std::string & filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file");
	}

	size_t filesize = (size_t)file.tellg();
	std::vector<char> buffer(filesize);

	file.seekg(0);
	file.read(buffer.data(), filesize);

	file.close();

	return buffer;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, 1);
	}
}

VKAPI_ATTR VkBool32 VKAPI_CALL vk_engine::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

// main loop involve rendering on the screen
void vk_engine::mainloop() {
	while (!glfwWindowShouldClose(_window)) {
		glfwPollEvents();
		drawFrame();
	}
	vkDeviceWaitIdle(_device);
}

void vk_engine::drawFrame() {
	// handle user's input
	float programTime = glfwGetTime();
	float frametime = (programTime - lastFrame) * 5.0f;
	lastFrame = programTime;

	double mouse_xpos;
	double mouse_ypos;

	glfwGetCursorPos(_window, &mouse_xpos, &mouse_ypos);
	_camera->updateCameraFront(mouse_xpos, mouse_ypos);

	_camera->updateCameraPos('m', frametime);

	if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
		_camera->updateCameraPos('w', frametime);
	if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
		_camera->updateCameraPos('a', frametime);
	if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
		_camera->updateCameraPos('s', frametime);
	if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
		_camera->updateCameraPos('d', frametime);

	vkWaitForFences(_device, 1, &_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, _imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
	vkResetCommandBuffer(_commandBuffers[imageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	// rerecord the command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional
	
	//clear depth at 1
	VkClearValue depthClear;
	depthClear.depthStencil.depth = 1.f;

	VkClearValue clearValues[] = { clearColor, depthClear };

	VK_CHECK(vkBeginCommandBuffer(_commandBuffers[imageIndex], &beginInfo));

	// set viewport and scissor dynamically
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)_swapChainExtent.width;
	viewport.height = (float)_swapChainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = _swapChainExtent;

	VkViewport viewports[] = { viewport };
	VkRect2D scissors[] = { scissor };

	vkCmdSetViewport(_commandBuffers[imageIndex], 0, 1, viewports);
	vkCmdSetScissor(_commandBuffers[imageIndex], 0, 1, scissors);

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = _renderpass;
	renderPassInfo.framebuffer = _swapChainFrameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = _swapChainExtent;
	renderPassInfo.clearValueCount = 2;
	renderPassInfo.pClearValues = &clearValues[0];

	vkCmdBeginRenderPass(_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	draw_objects(_commandBuffers[imageIndex], _renderables.data(), _renderables.size());

	vkCmdEndRenderPass(_commandBuffers[imageIndex]);

	VK_CHECK(vkEndCommandBuffer(_commandBuffers[imageIndex]));

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { _imageAvailableSemaphore[currentFrame] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &_commandBuffers[imageIndex];

	VkSemaphore signalSemaphores[] = { _renderFinishedSemaphore[currentFrame] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(_device, 1, &_inFlightFences[currentFrame]);
	VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _inFlightFences[currentFrame]));

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { _swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;
	presentInfo.pResults = nullptr; // Optional

	vkQueuePresentKHR(_presentQueue, &presentInfo);

	currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	_frameNumber += 1;
}

Material* vk_engine::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name) {
	Material mat;
	mat.pipeline = pipeline;
	mat.pipelineLayout = layout;
	_materials[name] = mat;
	return &_materials[name];
}

Material* vk_engine::get_material(const std::string& name) {
	if (_materials.find(name) != _materials.end()) {
		return &_materials[name];
	}
	else {
		return nullptr;
	}
}

Mesh* vk_engine::get_mesh(const std::string& name) {
	if (_meshes.find(name) != _meshes.end()) {
		return &_meshes[name];
	}
	else {
		return nullptr;
	}
}

void vk_engine::draw_objects(VkCommandBuffer cmd, RenderObject* first, int count) {
	Mesh* lastMesh = nullptr;
	Material* lastMaterial = nullptr;

	for (int i = 0; i < count; i++) {
		glm::mat4 mesh_matrix = _camera->modelViewMatrix() * first[i].transformMatrix;

		if (lastMaterial != first[i].material) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, first[i].material->pipeline);
			lastMaterial = first[i].material;
		}

		if (lastMesh != first[i].mesh) {
			// bind the mesh vertex buffer with offset 0
			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &first[i].mesh->_vertexBuffer._buffer, &offset);
			lastMesh = first[i].mesh;
		}

		MeshPushConsts consts;
		consts.render_matrix = mesh_matrix;

		// push the constant to the gpu
		vkCmdPushConstants(cmd, first[i].material->pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConsts), &consts);

		vkCmdDraw(cmd, first[i].mesh->_vertices.size(), 1, 0, 0);
	}
}

void vk_engine::load_meshes() {
	Mesh bus;
	if (!bus.load_from_obj("assets/gen-bus-2.obj")){
		std::cout << "failed to load obj!" << std::endl;
		abort();
	}

	_meshes["bus"] = bus;
	upload_mesh(_meshes["bus"]);
}

void vk_engine::upload_mesh(Mesh& mesh) {
	// allocate Vertex Buffer
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	// total size in bytes
	bufferInfo.size = mesh._vertices.size() * sizeof(Vertex);
	bufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;

	// let vma know this buffer is gonna written by cpu and read by gpu
	VmaAllocationCreateInfo allocationInfo{};
	allocationInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VK_CHECK(vmaCreateBuffer(_allocator, &bufferInfo, &allocationInfo, &mesh._vertexBuffer._buffer, &mesh._vertexBuffer._allocation, nullptr));

	_deletionQueue.push_function([&]() {
		vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
		});

	// copy vertex data
	void* data;
	vmaMapMemory(_allocator, mesh._vertexBuffer._allocation, &data);

	memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));

	vmaUnmapMemory(_allocator, mesh._vertexBuffer._allocation);
}

// run the engine
void vk_engine::run() {
	init_window();
	init_input();
	init_vulkan();
	init_scene();
	mainloop();
	cleanup();
}

// initialize the GLFW Window
void vk_engine::init_window() {
	if (!glfwInit())
		throw std::runtime_error("GLFW initialization failed!");

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	_window = glfwCreateWindow(WIDTH, HEIGHT, "vk_engine", NULL, NULL);
}

void vk_engine::init_input() {
	glfwSetKeyCallback(_window, key_callback);
	_camera = new Camera{ WIDTH, HEIGHT };
	glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

// initialize vulkan such as _instance, _device
void vk_engine::init_vulkan() {
	createInstance();
	createLogicalDevice();

	// initialize the memory allocator
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = _physicalDevice;
	allocatorInfo.device = _device;
	allocatorInfo.instance = _instance;
	vmaCreateAllocator(&allocatorInfo, &_allocator);
	
	_deletionQueue.push_function([&](){
		vmaDestroyAllocator(_allocator);
	});

	createSwapChain();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommands();
	createSyncObjects();

	load_meshes();
}

void vk_engine::init_scene() {
	/* RenderObject monkey;
	monkey.mesh = get_mesh("monkey");
	monkey.material = get_material("defaultmesh");
	monkey.transformMatrix = glm::mat4{ 1.0f };

	_renderables.push_back(monkey);

	monkey.mesh = get_mesh("left_monkey");
	monkey.transformMatrix = glm::translate(monkey.transformMatrix, glm::vec3(-3, 0, 0));
	_renderables.push_back(monkey);

	monkey.mesh = get_mesh("right_monkey");
	monkey.transformMatrix = glm::translate(monkey.transformMatrix, glm::vec3(6, 0, 0));
	_renderables.push_back(monkey);*/

	RenderObject bus;
	bus.mesh = get_mesh("bus");
	bus.material = get_material("defaultmesh");
	bus.transformMatrix = glm::mat4{ 1.0f };

	_renderables.push_back(bus);
}

void vk_engine::createInstance() {
	// check validation layers availability
	if (enableValidationLayers && !vk_support::checkValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	// instance info
	VkInstanceCreateInfo createInfo {};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;

	// get extensions
	std::vector<const char*> extensions = vk_support::getRequiredExtension(enableValidationLayers);
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = vk_init::DebugMessengerCreateInfo(debugCallback);

	// enable validation layers
	if (enableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
	}

	// create the _instance
	VK_CHECK(vkCreateInstance(&createInfo, nullptr, &_instance));

	_deletionQueue.push_function([=](){
		vkDestroyInstance(_instance, nullptr);
	});

	// setup debug messenger
	if (enableValidationLayers) {
		if (vk_support::CreateDebugUtilsMessengerEXT(_instance, &debugCreateInfo, nullptr, &_debugmessager) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
		_deletionQueue.push_function([=](){
			vk_support::DestroyDebugUtilsMessengerEXT(_instance, _debugmessager, nullptr);
		});
	}

	//create surface
	VK_CHECK(glfwCreateWindowSurface(_instance, _window, nullptr, &_surface));

	_deletionQueue.push_function([=](){
		vkDestroySurfaceKHR(_instance, _surface, nullptr);
	});
}

void vk_engine::createLogicalDevice() {
	// pick physical device
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

	if (deviceCount == 0) {
		throw std::runtime_error("no physical device available!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

	for (const auto& device : devices) {
		VkPhysicalDeviceProperties deviceProperties{};
		vkGetPhysicalDeviceProperties(device, &deviceProperties);
		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			_physicalDevice = device;
			break;
		}
	}

	if (_physicalDevice == VK_NULL_HANDLE) {
		throw std::runtime_error("no physical device available!");
	}

	// create logical device
	_indices = vk_support::findQueueFamilies(_physicalDevice, _surface);

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};
	std::set<uint32_t> uniqueQueueFamilies = { _indices.graphicFamily.value(), _indices.presentFamily.value() };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo = vk_init::DeviceQueueCreateInfo(queueFamily, queuePriority);
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};

	VkDeviceCreateInfo deviceCreateInfo = vk_init::DeviceCreateInfo(queueCreateInfos, deviceFeatures, deviceExtensions);

	VK_CHECK(vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device));

	_deletionQueue.push_function([=](){
		vkDestroyDevice(_device, nullptr);
	});

	// get queue
	vkGetDeviceQueue(_device, _indices.graphicFamily.value(), 0, &_graphicsQueue);
	vkGetDeviceQueue(_device, _indices.presentFamily.value(), 0, &_presentQueue);
}

void vk_engine::createSwapChain() {
	SwapChainSupportDetails swapChainSupport = querySwapChainSupport(_physicalDevice);
	VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
	VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
	VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

	_swapChainImageFormat = surfaceFormat.format;
	_swapChainExtent = extent;

	uint32_t image_count = swapChainSupport.capabilities.minImageCount + 1;

	if (swapChainSupport.capabilities.maxImageCount > 0 && image_count > swapChainSupport.capabilities.maxImageCount) {
		image_count = swapChainSupport.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = vk_init::SwapChainCreateInfo(_surface, image_count, swapChainSupport, surfaceFormat, presentMode, extent, _indices);

	VK_CHECK(vkCreateSwapchainKHR(_device, &createInfo, nullptr, &_swapChain));

	_deletionQueue.push_function([=](){
		vkDestroySwapchainKHR(_device, _swapChain, nullptr);
	});

	vkGetSwapchainImagesKHR(_device, _swapChain, &image_count, nullptr);
	_swapChainImages.resize(image_count);
	vkGetSwapchainImagesKHR(_device, _swapChain, &image_count, _swapChainImages.data());

	// create Image Views
	_swapChainImageViews.resize(_swapChainImages.size());

	for (size_t i = 0; i < _swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = vk_init::ImageViewCreateInfo(_swapChainImages[i], _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
		VK_CHECK(vkCreateImageView(_device, &createInfo, nullptr, &_swapChainImageViews[i]));
		_deletionQueue.push_function([=](){
			vkDestroyImageView(_device, _swapChainImageViews[i], nullptr);
		});
	}

	VkExtent3D depthImageExtent = {
		extent.width,
		extent.height,
		1
	};

	// hardcoding the format to 32 bit Float
	_depthFormat = VK_FORMAT_D32_SFLOAT;

	// the depth image will be an image with the format we selected and Depth Attachment usage flag
	VkImageCreateInfo dimg_info = vk_init::ImageCreateInfo(_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

	// for the depth image, we want to allocate it from GPU local memory
	VmaAllocationCreateInfo dimg_allocInfo{};
	dimg_allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	dimg_allocInfo.requiredFlags = VkMemoryPropertyFlags(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	// allocate and create the image
	vmaCreateImage(_allocator, &dimg_info, &dimg_allocInfo, &_depthImage._image, &_depthImage._allocation, nullptr);

	_deletionQueue.push_function([=](){
		vmaDestroyImage(_allocator, _depthImage._image, _depthImage._allocation);
	});

	// build an image-view for the depth image to use for rendering
	VkImageViewCreateInfo dview_info = vk_init::ImageViewCreateInfo(_depthImage._image, _depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

	VK_CHECK(vkCreateImageView(_device, &dview_info, nullptr, &_depthImageView));

	_deletionQueue.push_function([=](){
		vkDestroyImageView(_device, _depthImageView, nullptr);
	});
}

void vk_engine::createRenderPass() {
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format = _swapChainImageFormat;
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = _depthFormat;
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkAttachmentDescription attachments[] = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = vk_init::RenderPassCreateInfo(attachments, subpass, dependency);

	VK_CHECK(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderpass));

	_deletionQueue.push_function([=](){
		vkDestroyRenderPass(_device, _renderpass, nullptr);
	});
}

void vk_engine::createGraphicsPipeline() {
	auto vertShaderCode = readfile("shaders/vert.spv");
	auto fragShaderCode = readfile("shaders/frag.spv");

	VkShaderModule vertShaderModule = createShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = createShaderModule(fragShaderCode);

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module = vertShaderModule;
	vertShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module = fragShaderModule;
	fragShaderStageInfo.pName = "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	VkPipelineLayoutCreateInfo pipelineLayoutInfo = vk_init::PipelineLayoutCreateInfo();

	// setup push constants
	VkPushConstantRange push_const;
	// this const range start from beg and the size
	push_const.offset = 0;
	push_const.size = sizeof(MeshPushConsts);
	// this const is accessable only by the vertex shader
	push_const.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	pipelineLayoutInfo.pPushConstantRanges = &push_const;
	pipelineLayoutInfo.pushConstantRangeCount = 1;

	VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout));

	_deletionQueue.push_function([=](){
		vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
	});

	VertexInputDescription description = Vertex::get_vertex_description();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = vk_init::VertexInputStateCreateInfo(description.bindings, description.attributes);
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = vk_init::InputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

	VkDynamicState DynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	
	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.pNext =  nullptr;
	dynamicState.dynamicStateCount = 2;
	dynamicState.pDynamicStates = DynamicStates;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = nullptr;
	viewportState.scissorCount = 1;
	viewportState.pScissors = nullptr;

	VkPipelineRasterizationStateCreateInfo rasterizer = vk_init::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
	VkPipelineMultisampleStateCreateInfo multisampling = vk_init::MultisampleStateCreateInfo();
	VkPipelineColorBlendAttachmentState colorBlendAttachment = vk_init::ColorBlendAttachmentState();
	VkPipelineColorBlendStateCreateInfo colorBlending = vk_init::ColorBlendStateCreateInfo(colorBlendAttachment);

	VkPipelineDepthStencilStateCreateInfo DepthStencilState = vk_init::PipelineDepthStencilStateCreateInfo(true, true, VK_COMPARE_OP_LESS);

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = &DepthStencilState;
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = &dynamicState;
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = _renderpass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	VK_CHECK(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline));

	create_material(_graphicsPipeline, _pipelineLayout, "defaultmesh");

	_deletionQueue.push_function([=](){
		vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
	});

	vkDestroyShaderModule(_device, vertShaderModule, nullptr);
	vkDestroyShaderModule(_device, fragShaderModule, nullptr);
}

VkShaderModule vk_engine::createShaderModule(const std::vector<char>& code) {
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	
	VkShaderModule shaderModule;
	VK_CHECK(vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule));

	return shaderModule;
}

void vk_engine::createFrameBuffers() {
	_swapChainFrameBuffers.resize(_swapChainImageViews.size());
	VkFramebufferCreateInfo framebufferInfo = vk_init::FramebufferCreateInfo(_renderpass, _swapChainExtent);

	for (size_t i = 0; i < _swapChainImageViews.size(); i++) {
		VkImageView attachments[] = {
			_swapChainImageViews[i],
			_depthImageView
		};
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.attachmentCount = 2;

		VK_CHECK(vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainFrameBuffers[i]));
		_deletionQueue.push_function([=](){
			vkDestroyFramebuffer(_device, _swapChainFrameBuffers[i], nullptr);
		});
	}
}

void vk_engine::createCommands() {
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = _indices.graphicFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

	VK_CHECK(vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool));

	_deletionQueue.push_function([=](){
		vkDestroyCommandPool(_device, _commandPool, nullptr);
	});

	_commandBuffers.resize(_swapChainFrameBuffers.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = _commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = (uint32_t)_commandBuffers.size();

	VK_CHECK(vkAllocateCommandBuffers(_device, &allocInfo, _commandBuffers.data()));
}

void vk_engine::createSyncObjects() {
	_imageAvailableSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	_renderFinishedSemaphore.resize(MAX_FRAMES_IN_FLIGHT);
	_inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	// _imagesInFlight.resize(_swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphore[i]));
		VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphore[i]));
		VK_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]));
		_deletionQueue.push_function([=](){
			vkDestroySemaphore(_device, _imageAvailableSemaphore[i], nullptr);
			vkDestroySemaphore(_device, _renderFinishedSemaphore[i], nullptr);
			vkDestroyFence(_device, _inFlightFences[i], nullptr);
		});
	}
}

// cleanup memory after terminate the program
void vk_engine::cleanup() {
	_deletionQueue.flush();

	glfwDestroyWindow(_window);

	glfwTerminate();
}

SwapChainSupportDetails vk_engine::querySwapChainSupport(const VkPhysicalDevice& device) {
	SwapChainSupportDetails details;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _surface, &details.capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, nullptr);

	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _surface, &formatCount, details.formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _surface, &presentModeCount, details.presentModes.data());
	}

	return details;
}

VkSurfaceFormatKHR vk_engine::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			return availableFormat;
		}
	}
	return availableFormats[0];
}

VkPresentModeKHR vk_engine::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D vk_engine::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		int width, height;
		glfwGetFramebufferSize(_window, &width, &height);

		VkExtent2D actualExtent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}
