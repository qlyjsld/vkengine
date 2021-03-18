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

const VkClearValue clearColor = { 0.0f, 0.75f, 0.5f, 1.0f };

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

VKAPI_ATTR VkBool32 VKAPI_CALL vk_engine::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
	std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}

// run the engine
void vk_engine::run() {
	init_window();
	init_vulkan();
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

// initialize vulkan such as _instance, _device
void vk_engine::init_vulkan() {
	createInstance();
	createLogicalDevice();
	createSwapChain();
	createRenderPass();
	createGraphicsPipeline();
	createFrameBuffers();
	createCommands();
	createSyncObjects();

	// initialize the memory allocator
	VmaAllocatorCreateInfo allocatorInfo{};
	allocatorInfo.physicalDevice = _physicalDevice;
	allocatorInfo.device = _device;
	allocatorInfo.instance = _instance;
	vmaCreateAllocator(&allocatorInfo, &_allocator);

	load_meshes();
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

	// setup debug messenger
	if (enableValidationLayers) {
		if (vk_support::CreateDebugUtilsMessengerEXT(_instance, &debugCreateInfo, nullptr, &_debugmessager) != VK_SUCCESS) {
			throw std::runtime_error("failed to set up debug messenger!");
		}
	}

	//create surface
	VK_CHECK(glfwCreateWindowSurface(_instance, _window, nullptr, &_surface));
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

	vkGetSwapchainImagesKHR(_device, _swapChain, &image_count, nullptr);
	_swapChainImages.resize(image_count);
	vkGetSwapchainImagesKHR(_device, _swapChain, &image_count, _swapChainImages.data());

	// create Image Views
	_swapChainImageViews.resize(_swapChainImages.size());

	for (size_t i = 0; i < _swapChainImages.size(); i++) {
		VkImageViewCreateInfo createInfo = vk_init::ImageViewCreateInfo(_swapChainImages[i], _swapChainImageFormat);
		VK_CHECK(vkCreateImageView(_device, &createInfo, nullptr, &_swapChainImageViews[i]));
	}
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

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.srcAccessMask = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	VkRenderPassCreateInfo renderPassInfo = vk_init::RenderPassCreateInfo(colorAttachment, subpass, dependency);

	VK_CHECK(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderpass));
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

	VertexInputDescription description = Vertex::get_vertex_description();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo = vk_init::VertexInputStateCreateInfo(description.bindings, description.attributes);
	VkPipelineInputAssemblyStateCreateInfo inputAssembly = vk_init::InputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);

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

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer = vk_init::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
	VkPipelineMultisampleStateCreateInfo multisampling = vk_init::MultisampleStateCreateInfo();
	VkPipelineColorBlendAttachmentState colorBlendAttachment = vk_init::ColorBlendAttachmentState();
	VkPipelineColorBlendStateCreateInfo colorBlending = vk_init::ColorBlendStateCreateInfo(colorBlendAttachment);

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState = &vertexInputInfo;
	pipelineInfo.pInputAssemblyState = &inputAssembly;
	pipelineInfo.pViewportState = &viewportState;
	pipelineInfo.pRasterizationState = &rasterizer;
	pipelineInfo.pMultisampleState = &multisampling;
	pipelineInfo.pDepthStencilState = nullptr; // Optional
	pipelineInfo.pColorBlendState = &colorBlending;
	pipelineInfo.pDynamicState = nullptr; // Optional
	pipelineInfo.layout = _pipelineLayout;
	pipelineInfo.renderPass = _renderpass;
	pipelineInfo.subpass = 0;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
	pipelineInfo.basePipelineIndex = -1; // Optional

	VK_CHECK(vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_graphicsPipeline));

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
			_swapChainImageViews[i]
		};
		framebufferInfo.pAttachments = attachments;

		VK_CHECK(vkCreateFramebuffer(_device, &framebufferInfo, nullptr, &_swapChainFrameBuffers[i]));
	}
}

void vk_engine::createCommands() {
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.queueFamilyIndex = _indices.graphicFamily.value();
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // Optional

	VK_CHECK(vkCreateCommandPool(_device, &poolInfo, nullptr, &_commandPool));

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
	_imagesInFlight.resize(_swapChainImages.size(), VK_NULL_HANDLE);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_imageAvailableSemaphore[i]));
		VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &_renderFinishedSemaphore[i]));
		VK_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &_inFlightFences[i]));
	}
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
	vkWaitForFences(_device, 1, &_inFlightFences[currentFrame], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, _imageAvailableSemaphore[currentFrame], VK_NULL_HANDLE, &imageIndex);
	vkResetCommandBuffer(_commandBuffers[imageIndex], VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

	// Check if a previous frame is using this image
	if (_imagesInFlight[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(_device, 1, &_imagesInFlight[imageIndex], VK_TRUE, UINT64_MAX);
	}

	// Mark the image as now being in use by this frame
	_imagesInFlight[imageIndex] = _inFlightFences[currentFrame];

	// rerecord the command buffer
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = 0; // Optional
	beginInfo.pInheritanceInfo = nullptr; // Optional

	VK_CHECK(vkBeginCommandBuffer(_commandBuffers[imageIndex], &beginInfo));

	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = _renderpass;
	renderPassInfo.framebuffer = _swapChainFrameBuffers[imageIndex];
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = _swapChainExtent;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearColor;

	vkCmdBeginRenderPass(_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(_commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, _graphicsPipeline);

	// bind the mesh vertex buffer with offset 0
	VkDeviceSize offset = 0;
	vkCmdBindVertexBuffers(_commandBuffers[imageIndex], 0, 1, &_monkeyMesh._vertexBuffer._buffer, &offset);

	// modelView matrix
	// camera position
	glm::vec3 camPos = { 0.0f, 0.0f, -2.25f };

	glm::mat4 view = glm::translate(glm::mat4(1.0f), camPos);
	// camera projection
	glm::mat4 projection = glm::perspective(glm::radians(70.0f), WIDTH / HEIGHT, 0.1f, 200.0f);

	// model rotation
	// glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(_frameNumber * 0.05f), glm::vec3(1.0f, 1.0f, 1.0f));
	glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));

	model = glm::rotate(model, glm::radians(_frameNumber * 0.01f), glm::vec3(0.0f, 1.0f, 0.0f));

	glm::mat4 mesh_matrix = projection * view * model;

	MeshPushConsts consts;
	consts.render_matrix = mesh_matrix;

	// push the constant to the gpu
	vkCmdPushConstants(_commandBuffers[imageIndex], _pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(MeshPushConsts), &consts);

	vkCmdDraw(_commandBuffers[imageIndex], _monkeyMesh._vertices.size(), 1, 0, 0);

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

// cleanup memory after terminate the program
void vk_engine::cleanup() {
	vmaDestroyBuffer(_allocator, _monkeyMesh._vertexBuffer._buffer, _monkeyMesh._vertexBuffer._allocation);

	vmaDestroyBuffer(_allocator, _triangleMesh._vertexBuffer._buffer, _triangleMesh._vertexBuffer._allocation);

	vmaDestroyAllocator(_allocator);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(_device, _imageAvailableSemaphore[i], nullptr);
		vkDestroySemaphore(_device, _renderFinishedSemaphore[i], nullptr);
		vkDestroyFence(_device, _inFlightFences[i], nullptr);
	}

	vkDestroyCommandPool(_device, _commandPool, nullptr);

	for (auto& frambuffer : _swapChainFrameBuffers) {
		vkDestroyFramebuffer(_device, frambuffer, nullptr);
	}

	vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
	vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);

	vkDestroyRenderPass(_device, _renderpass, nullptr);

	for (auto& imageView : _swapChainImageViews) {
		vkDestroyImageView(_device, imageView, nullptr);
	}

	vkDestroySwapchainKHR(_device, _swapChain, nullptr);

	vkDestroyDevice(_device, nullptr);

	if (enableValidationLayers) {
		vk_support::DestroyDebugUtilsMessengerEXT(_instance, _debugmessager, nullptr);
	}

	vkDestroySurfaceKHR(_instance, _surface, nullptr);

	vkDestroyInstance(_instance, nullptr);

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

void vk_engine::load_meshes() {
	_triangleMesh._vertices.resize(3);

	_triangleMesh._vertices[0].position = { 0.0f, -0.5f, 0.0f };
	_triangleMesh._vertices[1].position = { 0.5f, 0.5f, 0.0f };
	_triangleMesh._vertices[2].position = { -0.5f, 0.5f, 0.0f };

	_triangleMesh._vertices[0].color = { 1.0f, 0.0f, 1.0f };
	_triangleMesh._vertices[1].color = { 0.0f, 1.0f, 0.0f };
	_triangleMesh._vertices[2].color = { 1.0f, 0.0f, 1.0f };

	_monkeyMesh.load_from_obj("assets/monkey_smooth.obj");
	
	upload_mesh(_triangleMesh);
	upload_mesh(_monkeyMesh);
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

	// copy vertex data
	void* data;
	vmaMapMemory(_allocator, mesh._vertexBuffer._allocation, &data);

	memcpy(data, mesh._vertices.data(), mesh._vertices.size() * sizeof(Vertex));

	vmaUnmapMemory(_allocator, mesh._vertexBuffer._allocation);
}