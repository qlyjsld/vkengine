#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstring>
#include <fstream>
#include <set>
#include <chrono>
#include <future>
#include <filesystem>

#define VMA_IMPLEMENTATION
#include "vk_engine/renderer/vk_renderer.h"
#include "vk_engine/renderer/vk_info.h"
#include "vk_engine/renderer/vk_texture.h"
#include "glm/gtc/matrix_transform.hpp"

#include "vk_engine/renderer/camera.h"
#include "vk_engine/core/logger.h"

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

namespace vk_engine {

	constexpr float WIDTH = 1600.0f;
	constexpr float HEIGHT = 900.0f;

	constexpr unsigned int MAX_FRAMES_IN_FLIGHT = 2;

	constexpr VkClearValue clearColor = { 0.25f, 0.25f, 0.25f, 1.0f };

	std::shared_ptr<spdlog::logger> logger::_corelogger;
	std::shared_ptr<spdlog::logger> logger::_clientlogger;

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

	// main loop involve rendering on the screen
	void vk_renderer::mainloop() {
		// auto indirectCommandsWorker = std::async(std::launch::async, [&]() {
			// while (!glfwWindowShouldClose(_window)) {
				// _drawSemaphore.acquire();

				VkDrawIndirectCommand* drawCommands;
				vmaMapMemory(_allocator, _indirectBuffer._allocation, (void**)&drawCommands);

				for (int i = 0; i < _renderables.size(); i++) {
					RenderObject& obj = _renderables[i];
					drawCommands[i].vertexCount = obj.mesh->_vertices.size();
					drawCommands[i].instanceCount = 1;
					drawCommands[i].firstVertex = 0;
					drawCommands[i].firstInstance = i;
				}

				vmaUnmapMemory(_allocator, _indirectBuffer._allocation);
			// }
		// });

		auto cpuToGpuWorker = std::async(std::launch::async, [&]() {
			while (!glfwWindowShouldClose(_window)) {
				_drawSemaphore.acquire();

				_cameraParameters.view = _camera->getViewMatrix();
				_cameraParameters.projection = _camera->getProjectionMatrix(WIDTH, HEIGHT);
				_cameraParameters.viewproj = _cameraParameters.projection * _cameraParameters.view;

				char* camdata;
				vmaMapMemory(_allocator, _cameraParametersBuffer._allocation, (void**)&camdata);
				camdata += pad_uniform_buffer_size(sizeof(GPUCameraData)) * _currentFrame;
				memcpy(camdata, &_cameraParameters, sizeof(GPUCameraData));
				vmaUnmapMemory(_allocator, _cameraParametersBuffer._allocation);

				char* sceneData;
				vmaMapMemory(_allocator, _sceneParametersBuffer._allocation, (void**)&sceneData);
				sceneData += pad_uniform_buffer_size(sizeof(GPUSceneData)) * _currentFrame;
				memcpy(sceneData, &_sceneParameters, sizeof(GPUSceneData));
				vmaUnmapMemory(_allocator, _sceneParametersBuffer._allocation);

				void* objData;
				vmaMapMemory(_allocator, _frames[_currentFrame]._objectBuffer._allocation, &objData);

				GPUObjectData* objectSSBO = (GPUObjectData*)objData;

				for (int i = 0; i < _renderables.size(); i++) {
					RenderObject& object = _renderables[i];
					objectSSBO[i].modelMatrix = object.transformMatrix;
				}

				vmaUnmapMemory(_allocator, _frames[_currentFrame]._objectBuffer._allocation);
			}
		});

		while (!glfwWindowShouldClose(_window)) {
			glfwPollEvents();

			// handle user's input
			float programTime = glfwGetTime();
			float frametime = (programTime - _lastFrame) * 5.0f;
			_lastFrame = programTime;

			double mouse_xpos;
			double mouse_ypos;

			glfwGetCursorPos(_window, &mouse_xpos, &mouse_ypos);
			_camera->updateCameraFront(mouse_xpos, mouse_ypos);

			if (glfwGetKey(_window, GLFW_KEY_W) == GLFW_PRESS)
				_camera->updateCameraPos('w', frametime);
			if (glfwGetKey(_window, GLFW_KEY_A) == GLFW_PRESS)
				_camera->updateCameraPos('a', frametime);
			if (glfwGetKey(_window, GLFW_KEY_S) == GLFW_PRESS)
				_camera->updateCameraPos('s', frametime);
			if (glfwGetKey(_window, GLFW_KEY_D) == GLFW_PRESS)
				_camera->updateCameraPos('d', frametime);

			drawFrame();
		}

		// indirectCommandsWorker.wait();
		cpuToGpuWorker.wait();
		vkDeviceWaitIdle(_device);
	}

	void vk_renderer::drawFrame() {
		// auto start = std::chrono::steady_clock::now();
		vkWaitForFences(_device, 1, &_frames[_currentFrame]._inFlightFences, VK_TRUE, UINT64_MAX);
		vkResetFences(_device, 1, &_frames[_currentFrame]._inFlightFences);

		// start memcpy to gpu
		_drawSemaphore.release();
		// _drawSemaphore.release();

		uint32_t imageIndex;
		vkAcquireNextImageKHR(_device, _swapChain, UINT64_MAX, _frames[_currentFrame]._imageAvailableSemaphore, VK_NULL_HANDLE, &imageIndex);
		vkResetCommandBuffer(_frames[_currentFrame]._maincommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);
		vkResetCommandBuffer(_frames[_currentFrame]._secondaryCommandBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT);

		// rerecord the command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT; // optional
		beginInfo.pInheritanceInfo = nullptr; // optional

		VK_CHECK(vkBeginCommandBuffer(_frames[_currentFrame]._maincommandBuffer, &beginInfo));

		// clear depth at 1
		VkClearValue depthClear;
		depthClear.depthStencil.depth = 1.f;

		VkClearValue clearValues[] = { clearColor, depthClear };

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

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = _renderpass;
		renderPassInfo.framebuffer = _swapChainFrameBuffers[imageIndex];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = _swapChainExtent;
		renderPassInfo.clearValueCount = 2;
		renderPassInfo.pClearValues = &clearValues[0];

		vkCmdBeginRenderPass(_frames[_currentFrame]._maincommandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		std::vector<IndirectBatch> batches = compactDraw(_renderables.data(), _renderables.size());

		for (const auto& batch : batches) {
			auto secondaryBufferRecording = std::async(std::launch::async, [&]() {
				draw_object(_frames[_currentFrame]._secondaryCommandBuffer, batch, _frames[_currentFrame], _renderpass, _swapChainFrameBuffers[imageIndex], viewports, scissors);
			});

			secondaryBufferRecording.wait();
		}

		vkCmdExecuteCommands(_frames[_currentFrame]._maincommandBuffer, 1, &_frames[_currentFrame]._secondaryCommandBuffer);

		vkCmdEndRenderPass(_frames[_currentFrame]._maincommandBuffer);

		VK_CHECK(vkEndCommandBuffer(_frames[_currentFrame]._maincommandBuffer));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { _frames[_currentFrame]._imageAvailableSemaphore };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &_frames[_currentFrame]._maincommandBuffer;

		VkSemaphore signalSemaphores[] = { _frames[_currentFrame]._renderFinishedSemaphore };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _frames[_currentFrame]._inFlightFences));

		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { _swapChain };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;
		presentInfo.pImageIndices = &imageIndex;
		presentInfo.pResults = nullptr; // optional

		vkQueuePresentKHR(_presentQueue, &presentInfo);

		_currentFrame = 1 - _currentFrame;
		_frameNumber += 1;

		/* auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		std::cout << "frame time: " << elapsed_seconds.count() * 100 << "ms\n"; */
	}

	void vk_renderer::draw_objects(VkCommandBuffer cmd, RenderObject* first, int count, const FrameData& frame, VkRenderPass renderPass, VkFramebuffer framebuffer) {

		VkCommandBufferInheritanceInfo commandBufferInheritanceInfo{};
		commandBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

		commandBufferInheritanceInfo.renderPass = renderPass;
		commandBufferInheritanceInfo.subpass = 0;
		commandBufferInheritanceInfo.framebuffer = framebuffer;

		// rerecord the command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT; // optional
		beginInfo.pInheritanceInfo = &commandBufferInheritanceInfo; // optional

		VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

		std::vector<IndirectBatch> draws = compactDraw(first, count);

		for (const auto& draw : draws) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipeline);

			uint32_t uniform_offset[] = { pad_uniform_buffer_size(sizeof(GPUCameraData)) * _currentFrame, pad_uniform_buffer_size(sizeof(GPUSceneData)) * _currentFrame };
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipelineLayout, 0, 1, &_globalDescriptor, 2, uniform_offset);

			//object data descriptor
			vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, draw.material->pipelineLayout, 1, 1, &frame._objectDescriptor, 0, nullptr);

			VkDeviceSize offset = 0;
			vkCmdBindVertexBuffers(cmd, 0, 1, &draw.mesh->_vertexBuffer._buffer, &offset);

			VkDeviceSize indirectOffset = draw.first * sizeof(VkDrawIndirectCommand);
			uint32_t drawStride = sizeof(VkDrawIndirectCommand);

			vkCmdDrawIndirect(cmd, _indirectBuffer._buffer, indirectOffset, draw.count, drawStride);
		}

		VK_CHECK(vkEndCommandBuffer(cmd));
	}

	void vk_renderer::draw_object(VkCommandBuffer cmd, const IndirectBatch& batch, const FrameData& frame, VkRenderPass renderPass, VkFramebuffer framebuffer, VkViewport* viewports, VkRect2D* scissors) {

		VkCommandBufferInheritanceInfo commandBufferInheritanceInfo{};
		commandBufferInheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

		commandBufferInheritanceInfo.renderPass = renderPass;
		commandBufferInheritanceInfo.subpass = 0;
		commandBufferInheritanceInfo.framebuffer = framebuffer;

		// rerecord the command buffer
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT; // optional
		beginInfo.pInheritanceInfo = &commandBufferInheritanceInfo; // optional

		VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

		vkCmdSetViewport(_frames[_currentFrame]._secondaryCommandBuffer, 0, 1, viewports);
		vkCmdSetScissor(_frames[_currentFrame]._secondaryCommandBuffer, 0, 1, scissors);

		vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, batch.material->pipeline);

		uint32_t uniform_offset[] = { pad_uniform_buffer_size(sizeof(GPUCameraData)) * _currentFrame, pad_uniform_buffer_size(sizeof(GPUSceneData)) * _currentFrame };
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, batch.material->pipelineLayout, 0, 1, &_globalDescriptor, 2, uniform_offset);

		//object data descriptor
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, batch.material->pipelineLayout, 1, 1, &frame._objectDescriptor, 0, nullptr);

		VkDeviceSize offset = 0;
		vkCmdBindVertexBuffers(cmd, 0, 1, &batch.mesh->_vertexBuffer._buffer, &offset);

		VkDeviceSize indirectOffset = batch.first * sizeof(VkDrawIndirectCommand);
		uint32_t drawStride = sizeof(VkDrawIndirectCommand);

		vkCmdDrawIndirect(cmd, _indirectBuffer._buffer, indirectOffset, batch.count, drawStride);

		VK_CHECK(vkEndCommandBuffer(cmd));
	}

	std::vector<IndirectBatch> vk_renderer::compactDraw(RenderObject* objs, int count) {
		std::vector<IndirectBatch> draws;

		IndirectBatch draw;
		draw.mesh = objs[0].mesh;
		draw.material = objs[0].material;
		draw.first = 0;
		draw.count = 1;

		draws.push_back(draw);

		for (int i = 1; i < count; i++) {
			if (objs[i].mesh == draws.back().mesh && objs[i].material == draws.back().material) {
				draws.back().count++;
			}
			else {
				IndirectBatch newdraw;
				newdraw.mesh = objs[i].mesh;
				newdraw.material = objs[i].material;
				newdraw.first = i;
				newdraw.count = 1;

				draws.push_back(newdraw);
			}
		}

		return draws;
	}

	// run the engine
	void vk_renderer::run() {
		logger::init();
		init_window();
		VK_LOG_INFO("GLFW window initialized successfully");
		init_input();
		init_vulkan();
		VK_LOG_INFO("Vulkan instance initialized successfully");
		init_scene();
		VK_LOG_INFO("Scene loaded successfully");
		VK_LOG_INFO("Start rendering");
		mainloop();
		cleanup();
	}

	// initialize the GLFW Window
	void vk_renderer::init_window() {
		if (!glfwInit())
			throw std::runtime_error("GLFW initialization failed!");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		_window = glfwCreateWindow(WIDTH, HEIGHT, "vk_renderer", NULL, NULL);
	}

	void vk_renderer::init_input() {
		glfwSetKeyCallback(_window, key_callback);
		_camera = new Camera;
		glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	// initialize vulkan such as _instance, _device
	void vk_renderer::init_vulkan() {
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
		createDescriptors();
		createRenderPass();
		createTexturelessPipeline();
		createGraphicsPipeline();
		createFrameBuffers();
		createCommands();
		createSyncObjects();
	}

	void vk_renderer::init_scene() {
		VK_LOG_INFO("Loading meshes...");

		auto start = std::chrono::steady_clock::now();
		load_meshes();
		auto end = std::chrono::steady_clock::now();
		std::chrono::duration<double> elapsed_seconds = end - start;
		std::cout << "decompress time: " << elapsed_seconds.count() << "s\n";

		// VK_LOG_INFO("Loading textures...");
		// load_textures();

		RenderObject interior;
		interior.mesh = get_mesh("assets/Interior/interior.asset");
		interior.material = get_material("texturelessMesh");
		interior.transformMatrix = glm::scale(glm::mat4{ 1.0f }, glm::vec3(0.05f, 0.05f, 0.05f));

		RenderObject exterior;
		exterior.mesh = get_mesh("assets/Exterior/exterior.asset");
		exterior.material = get_material("texturelessMesh");
		exterior.transformMatrix = glm::scale(glm::mat4{ 1.0f }, glm::vec3(0.05f, 0.05f, 0.05f));

		std::cout << "vertices: " << _meshes["assets/Interior/interior.asset"]._vertices.size() << std::endl;
		std::cout << "vertices: " << _meshes["assets/Exterior/exterior.asset"]._vertices.size() << std::endl;

		_renderables.push_back(interior);
		_renderables.push_back(exterior);

		/* VkSamplerCreateInfo samplerInfo = vk_info::SamplerCreateInfo(VK_FILTER_NEAREST, VK_SAMPLER_ADDRESS_MODE_REPEAT);

		VkSampler blockySampler{};
		vkCreateSampler(_device, &samplerInfo, nullptr, &blockySampler);

		_deletionQueue.push_function([=]() {
			vkDestroySampler(_device, blockySampler, nullptr);
		});

		VkDescriptorSetAllocateInfo allocInfo = vk_info::DescriptorSetAllocateInfo(_descriptorPool, _textureSetLayout);
		vkAllocateDescriptorSets(_device, &allocInfo, &San_Miguel.material->textureSet);

		VkDescriptorImageInfo imageBufferInfo = vk_info::DescriptorImageInfo(blockySampler, _textures["San_Miguel"].imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		VkWriteDescriptorSet texture1 = vk_info::WriteDescriptorSetImage(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, San_Miguel.material->textureSet, &imageBufferInfo, 0);

		vkUpdateDescriptorSets(_device, 1, &texture1, 0, nullptr); */
	}

	void vk_renderer::createInstance() {
		// check validation layers availability
		if (enableValidationLayers && !vk_support::checkValidationLayerSupport()) {
			throw std::runtime_error("validation layers requested, but not available!");
		}

		VkApplicationInfo appInfo{};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = nullptr;
		appInfo.apiVersion = VK_API_VERSION_1_2;

		// instance info
		VkInstanceCreateInfo createInfo {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pApplicationInfo = &appInfo;

		// get extensions
		std::vector<const char*> extensions = vk_support::getRequiredExtension(enableValidationLayers);
		createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		createInfo.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = vk_info::DebugMessengerCreateInfo(debugCallback);

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

	void vk_renderer::createLogicalDevice() {
		// pick physical device
		uint32_t deviceCount = 0;
		vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);

		if (deviceCount == 0) {
			throw std::runtime_error("no physical device available!");
		}

		std::vector<VkPhysicalDevice> devices(deviceCount);
		vkEnumeratePhysicalDevices(_instance, &deviceCount, devices.data());

		for (const auto& device : devices) {
			vkGetPhysicalDeviceProperties(device, &_deviceProperties);
			if (_deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
				_physicalDevice = device;
				VK_LOG_INFO("The GPU has a minimum buffer alignment of " + std::to_string(_deviceProperties.limits.minUniformBufferOffsetAlignment));
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
			VkDeviceQueueCreateInfo queueCreateInfo = vk_info::DeviceQueueCreateInfo(queueFamily, queuePriority);
			queueCreateInfos.push_back(queueCreateInfo);
		}

		VkPhysicalDeviceFeatures deviceFeatures{};

		VkDeviceCreateInfo deviceCreateInfo = vk_info::DeviceCreateInfo(queueCreateInfos, deviceFeatures, deviceExtensions);

		VkPhysicalDeviceShaderDrawParametersFeatures deviceDrawParametersInfo{};
		deviceDrawParametersInfo.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_DRAW_PARAMETERS_FEATURES;
		deviceDrawParametersInfo.pNext = nullptr;
		deviceDrawParametersInfo.shaderDrawParameters = VK_TRUE;

		deviceCreateInfo.pNext = &deviceDrawParametersInfo;

		VK_CHECK(vkCreateDevice(_physicalDevice, &deviceCreateInfo, nullptr, &_device));

		_deletionQueue.push_function([=](){
			vkDestroyDevice(_device, nullptr);
		});

		// get queue
		vkGetDeviceQueue(_device, _indices.graphicFamily.value(), 0, &_graphicsQueue);
		vkGetDeviceQueue(_device, _indices.presentFamily.value(), 0, &_presentQueue);
	}

	void vk_renderer::createSwapChain() {
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

		VkSwapchainCreateInfoKHR createInfo = vk_info::SwapChainCreateInfo(_surface, image_count, swapChainSupport, surfaceFormat, presentMode, extent, _indices);

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
			VkImageViewCreateInfo createInfo = vk_info::ImageViewCreateInfo(_swapChainImages[i], _swapChainImageFormat, VK_IMAGE_ASPECT_COLOR_BIT);
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
		VkImageCreateInfo dimg_info = vk_info::ImageCreateInfo(_depthFormat, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, depthImageExtent);

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
		VkImageViewCreateInfo dview_info = vk_info::ImageViewCreateInfo(_depthImage._image, _depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);

		VK_CHECK(vkCreateImageView(_device, &dview_info, nullptr, &_depthImageView));

		_deletionQueue.push_function([=](){
			vkDestroyImageView(_device, _depthImageView, nullptr);
		});
	}

	void vk_renderer::createDescriptors() {
		// create indirect buffer
		_indirectBuffer = create_buffer(FRAME_OVERLAP * sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		_deletionQueue.push_function([=]() {
			vmaDestroyBuffer(_allocator, _indirectBuffer._buffer, _indirectBuffer._allocation);
		});

		// create a descriptor pool that will hold 10 uniform buffers
		std::vector<VkDescriptorPoolSize> sizes = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 4 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4}
		};

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.pNext = nullptr;

		poolInfo.flags = 0;
		poolInfo.maxSets = 10;
		poolInfo.poolSizeCount = (uint32_t)sizes.size();
		poolInfo.pPoolSizes = sizes.data();

		VK_CHECK(vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool));

		_deletionQueue.push_function([&]() {
			vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
		});

		size_t sceneParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));
		_sceneParametersBuffer = create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		_deletionQueue.push_function([=]() {
			vmaDestroyBuffer(_allocator, _sceneParametersBuffer._buffer, _sceneParametersBuffer._allocation);
		});

		size_t cameraParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUCameraData));
		_cameraParametersBuffer = create_buffer(cameraParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		_deletionQueue.push_function([=]() {
			vmaDestroyBuffer(_allocator, _cameraParametersBuffer._buffer, _cameraParametersBuffer._allocation);
		});

		// information about the binding
		VkDescriptorSetLayoutBinding camBind = vk_info::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_VERTEX_BIT, 0);
		VkDescriptorSetLayoutBinding sceneBind = vk_info::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_FRAGMENT_BIT, 1);

		VkDescriptorSetLayoutBinding bindings[] = { camBind, sceneBind };

		VkDescriptorSetLayoutCreateInfo set1Info{};
		set1Info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set1Info.pNext = nullptr;

		set1Info.bindingCount = 2;
		set1Info.flags = 0;
		set1Info.pBindings = bindings;

		VK_CHECK(vkCreateDescriptorSetLayout(_device, &set1Info, nullptr, &_globalSetLayout));

		_deletionQueue.push_function([&]() {
			vkDestroyDescriptorSetLayout(_device, _globalSetLayout, nullptr);
			});

		// allocate one descriptor set for each frame
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;

		allocInfo.descriptorPool = _descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &_globalSetLayout;

		vkAllocateDescriptorSets(_device, &allocInfo, &_globalDescriptor);

		// information about the buffer we want to point at in the descriptor
		VkDescriptorBufferInfo cambinfo{};
		cambinfo.buffer = _cameraParametersBuffer._buffer;
		cambinfo.offset = 0;
		cambinfo.range = sizeof(GPUCameraData);

		VkDescriptorBufferInfo scenebinfo{};
		scenebinfo.buffer = _sceneParametersBuffer._buffer;
		scenebinfo.offset = 0;
		scenebinfo.range = sizeof(GPUSceneData);

		VkWriteDescriptorSet camwrite = vk_info::WriteDescriptorSetBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _globalDescriptor, &cambinfo, 0);
		VkWriteDescriptorSet scenewrite = vk_info::WriteDescriptorSetBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, _globalDescriptor, &scenebinfo, 1);

		VkDescriptorSetLayoutBinding objBind = vk_info::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0);

		VkDescriptorSetLayoutCreateInfo set2Info{};
		set2Info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set2Info.pNext = nullptr;

		set2Info.bindingCount = 1;
		set2Info.flags = 0;
		set2Info.pBindings = &objBind;

		VK_CHECK(vkCreateDescriptorSetLayout(_device, &set2Info, nullptr, &_objectSetLayout));

		_deletionQueue.push_function([&]() {
			vkDestroyDescriptorSetLayout(_device, _objectSetLayout, nullptr);
		});

		for (int i = 0; i < FRAME_OVERLAP; i++) {
			const int MAX_OBJECTS = 32767;
			_frames[i]._objectBuffer = create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

			_deletionQueue.push_function([=]() {
				vmaDestroyBuffer(_allocator, _frames[i]._objectBuffer._buffer, _frames[i]._objectBuffer._allocation);
			});

			// allocate one descriptor set for each frame
			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;

			allocInfo.descriptorPool = _descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &_objectSetLayout;

			vkAllocateDescriptorSets(_device, &allocInfo, &_frames[i]._objectDescriptor);

			VkDescriptorBufferInfo objbinfo{};
			objbinfo.buffer = _frames[i]._objectBuffer._buffer;
			objbinfo.offset = 0;
			objbinfo.range = sizeof(GPUObjectData) * MAX_OBJECTS;

			VkWriteDescriptorSet objwrite = vk_info::WriteDescriptorSetBuffer(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, _frames[i]._objectDescriptor, &objbinfo, 0);

			VkWriteDescriptorSet setwrites[] = { camwrite, scenewrite, objwrite };
			vkUpdateDescriptorSets(_device, 3, setwrites, 0, nullptr);
		}

		VkDescriptorSetLayoutBinding texBind = vk_info::DescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 0);

		VkDescriptorSetLayoutCreateInfo set3Info{};
		set3Info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		set3Info.pNext = nullptr;

		set3Info.bindingCount = 1;
		set3Info.flags = 0;
		set3Info.pBindings = &texBind;

		VK_CHECK(vkCreateDescriptorSetLayout(_device, &set3Info, nullptr, &_textureSetLayout));

		_deletionQueue.push_function([&]() {
			vkDestroyDescriptorSetLayout(_device, _textureSetLayout, nullptr);
		});
	}

	void vk_renderer::createRenderPass() {
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
		VkRenderPassCreateInfo renderPassInfo = vk_info::RenderPassCreateInfo(attachments, subpass, dependency);

		VK_CHECK(vkCreateRenderPass(_device, &renderPassInfo, nullptr, &_renderpass));

		_deletionQueue.push_function([=](){
			vkDestroyRenderPass(_device, _renderpass, nullptr);
		});
	}

	VkPipeline vk_engine::PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass) {
		VkPipelineColorBlendStateCreateInfo colorBlending = vk_info::ColorBlendStateCreateInfo(_colorBlendAttachment);

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = _shaderStages;
		pipelineInfo.pVertexInputState = &_vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &_inputAssembly;
		pipelineInfo.pViewportState = &_viewportState;
		pipelineInfo.pRasterizationState = &_rasterizer;
		pipelineInfo.pMultisampleState = &_multisampling;
		pipelineInfo.pDepthStencilState = &_depthStencil;
		pipelineInfo.pColorBlendState = &colorBlending;
		pipelineInfo.pDynamicState = &_dynamicState;
		pipelineInfo.layout = _pipelineLayout;
		pipelineInfo.renderPass = pass;
		pipelineInfo.subpass = 0;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // optional
		pipelineInfo.basePipelineIndex = -1; // optional

		VkPipeline pipeline{};
		VK_CHECK(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));

		return pipeline;
	}

	void vk_renderer::createGraphicsPipeline() {
		PipelineBuilder graphic_pipeline_info{};

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

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = vk_info::PipelineLayoutCreateInfo();

		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		// hook the global set layout
		VkDescriptorSetLayout setLayouts[] = { _globalSetLayout, _objectSetLayout, _textureSetLayout };
		pipelineLayoutInfo.setLayoutCount = 3;
		pipelineLayoutInfo.pSetLayouts = setLayouts;

		VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipelineLayout));

		_deletionQueue.push_function([=](){
			vkDestroyPipelineLayout(_device, _pipelineLayout, nullptr);
		});

		VertexInputDescription description = Vertex::get_vertex_description();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = vk_info::VertexInputStateCreateInfo(description.bindings, description.attributes);
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = vk_info::InputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
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

		VkPipelineRasterizationStateCreateInfo rasterizer = vk_info::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
		VkPipelineMultisampleStateCreateInfo multisampling = vk_info::MultisampleStateCreateInfo();
		VkPipelineColorBlendAttachmentState colorBlendAttachment = vk_info::ColorBlendAttachmentState();
		VkPipelineDepthStencilStateCreateInfo DepthStencilState = vk_info::PipelineDepthStencilStateCreateInfo(true, true, VK_COMPARE_OP_LESS);

		graphic_pipeline_info._shaderStages = shaderStages;
		graphic_pipeline_info._vertexInputInfo = vertexInputInfo;
		graphic_pipeline_info._inputAssembly = inputAssembly;
		graphic_pipeline_info._viewportState = viewportState;
		graphic_pipeline_info._rasterizer = rasterizer;
		graphic_pipeline_info._multisampling = multisampling;
		graphic_pipeline_info._depthStencil = DepthStencilState;
		graphic_pipeline_info._colorBlendAttachment = colorBlendAttachment;
		graphic_pipeline_info._dynamicState = dynamicState;
		graphic_pipeline_info._pipelineLayout = _pipelineLayout;
		_graphicsPipeline = graphic_pipeline_info.build_pipeline(_device, _renderpass);

		create_material(_graphicsPipeline, _pipelineLayout, "defaultMesh");

		_deletionQueue.push_function([=](){
			vkDestroyPipeline(_device, _graphicsPipeline, nullptr);
		});

		vkDestroyShaderModule(_device, vertShaderModule, nullptr);
		vkDestroyShaderModule(_device, fragShaderModule, nullptr);
	}

	void vk_renderer::createTexturelessPipeline() {
		PipelineBuilder graphic_pipeline_info{};

		auto vertShaderCode = readfile("shaders/textureless_mesh.spv");
		auto fragShaderCode = readfile("shaders/textureless.spv");
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

		VkPipelineLayoutCreateInfo pipelineLayoutInfo = vk_info::PipelineLayoutCreateInfo();

		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		// hook the global set layout
		VkDescriptorSetLayout setLayouts[] = { _globalSetLayout, _objectSetLayout };
		pipelineLayoutInfo.setLayoutCount = 2;
		pipelineLayoutInfo.pSetLayouts = setLayouts;

		VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_texturelesspipelineLayout));

		_deletionQueue.push_function([=]() {
			vkDestroyPipelineLayout(_device, _texturelesspipelineLayout, nullptr);
			});

		VertexInputDescription description = Vertex::get_vertex_description();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo = vk_info::VertexInputStateCreateInfo(description.bindings, description.attributes);
		VkPipelineInputAssemblyStateCreateInfo inputAssembly = vk_info::InputAssemblyStateCreateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
		VkDynamicState DynamicStates[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicState{};
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.pNext = nullptr;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = DynamicStates;

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.viewportCount = 1;
		viewportState.pViewports = nullptr;
		viewportState.scissorCount = 1;
		viewportState.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterizer = vk_info::RasterizationStateCreateInfo(VK_POLYGON_MODE_FILL);
		VkPipelineMultisampleStateCreateInfo multisampling = vk_info::MultisampleStateCreateInfo();
		VkPipelineColorBlendAttachmentState colorBlendAttachment = vk_info::ColorBlendAttachmentState();
		VkPipelineDepthStencilStateCreateInfo DepthStencilState = vk_info::PipelineDepthStencilStateCreateInfo(true, true, VK_COMPARE_OP_LESS);

		graphic_pipeline_info._shaderStages = shaderStages;
		graphic_pipeline_info._vertexInputInfo = vertexInputInfo;
		graphic_pipeline_info._inputAssembly = inputAssembly;
		graphic_pipeline_info._viewportState = viewportState;
		graphic_pipeline_info._rasterizer = rasterizer;
		graphic_pipeline_info._multisampling = multisampling;
		graphic_pipeline_info._depthStencil = DepthStencilState;
		graphic_pipeline_info._colorBlendAttachment = colorBlendAttachment;
		graphic_pipeline_info._dynamicState = dynamicState;
		graphic_pipeline_info._pipelineLayout = _texturelesspipelineLayout;
		_texturelesPipeline = graphic_pipeline_info.build_pipeline(_device, _renderpass);

		create_material(_texturelesPipeline, _texturelesspipelineLayout, "texturelessMesh");

		_deletionQueue.push_function([=]() {
			vkDestroyPipeline(_device, _texturelesPipeline, nullptr);
			});

		vkDestroyShaderModule(_device, vertShaderModule, nullptr);
		vkDestroyShaderModule(_device, fragShaderModule, nullptr);
	}

	VkShaderModule vk_renderer::createShaderModule(const std::vector<char>& code) {
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	
		VkShaderModule shaderModule;
		VK_CHECK(vkCreateShaderModule(_device, &createInfo, nullptr, &shaderModule));

		return shaderModule;
	}

	void vk_renderer::createFrameBuffers() {
		_swapChainFrameBuffers.resize(_swapChainImageViews.size());
		VkFramebufferCreateInfo framebufferInfo = vk_info::FramebufferCreateInfo(_renderpass, _swapChainExtent);

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

	void vk_renderer::createCommands() {
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = _indices.graphicFamily.value();
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT; // optional

		for (auto& frame : _frames) {
			VK_CHECK(vkCreateCommandPool(_device, &poolInfo, nullptr, &frame._maincommandPool));
			VK_CHECK(vkCreateCommandPool(_device, &poolInfo, nullptr, &frame._secondaryCommandPool));

			_deletionQueue.push_function([=]() {
				vkDestroyCommandPool(_device, frame._maincommandPool, nullptr);
				vkDestroyCommandPool(_device, frame._secondaryCommandPool, nullptr);
			});

			VkCommandBufferAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = frame._maincommandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			allocInfo.commandBufferCount = 1;

			VK_CHECK(vkAllocateCommandBuffers(_device, &allocInfo, &frame._maincommandBuffer));

			allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			allocInfo.commandPool = frame._secondaryCommandPool;
			allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			allocInfo.commandBufferCount = 1;

			VK_CHECK(vkAllocateCommandBuffers(_device, &allocInfo, &frame._secondaryCommandBuffer));
		}

		VK_CHECK(vkCreateCommandPool(_device, &poolInfo, nullptr, &_uploadContext._commandPool));

		_deletionQueue.push_function([=]() {
			vkDestroyCommandPool(_device, _uploadContext._commandPool, nullptr);
		});
	}

	void vk_renderer::createSyncObjects() {
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (auto& frame : _frames) {
			VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &frame._imageAvailableSemaphore));
			VK_CHECK(vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &frame._renderFinishedSemaphore));
			VK_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &frame._inFlightFences));
			_deletionQueue.push_function([=](){
				vkDestroySemaphore(_device, frame._imageAvailableSemaphore, nullptr);
				vkDestroySemaphore(_device, frame._renderFinishedSemaphore, nullptr);
				vkDestroyFence(_device, frame._inFlightFences, nullptr);
			});
		}

		fenceInfo.flags = NULL;

		VK_CHECK(vkCreateFence(_device, &fenceInfo, nullptr, &_uploadContext._uploadFence));

		_deletionQueue.push_function([=](){
			vkDestroyFence(_device, _uploadContext._uploadFence, nullptr);
		});
	}

	// cleanup memory after terminate the program
	void vk_renderer::cleanup() {
		_deletionQueue.flush();

		glfwDestroyWindow(_window);

		glfwTerminate();
	}

	AllocatedBuffer vk_renderer::create_buffer(size_t allocSize, VkBufferUsageFlags usage, VmaMemoryUsage memoryUsage) {
		// allocate vertex buffer
		VkBufferCreateInfo bufferinfo{};
		bufferinfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferinfo.pNext = nullptr;

		bufferinfo.size = allocSize;
		bufferinfo.usage = usage;

		VmaAllocationCreateInfo vmaallocInfo{};
		vmaallocInfo.usage = memoryUsage;

		AllocatedBuffer newbuffer;
		VK_CHECK(vmaCreateBuffer(_allocator, &bufferinfo, &vmaallocInfo, &newbuffer._buffer, &newbuffer._allocation, nullptr));

		return newbuffer;
	}

	size_t vk_renderer::pad_uniform_buffer_size(size_t originalSize) {
		size_t minUboAlignment = _deviceProperties.limits.minUniformBufferOffsetAlignment;
		size_t alignedSize = originalSize;
		if (minUboAlignment > 0) {
			alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
		return alignedSize;
	}

	Mesh* vk_renderer::get_mesh(const std::string& name) {
		if (_meshes.find(name) != _meshes.end()) {
			return &_meshes[name];
		}
		else {
			return nullptr;
		}
	}

	Material* vk_renderer::get_material(const std::string& name) {
		if (_materials.find(name) != _materials.end()) {
			return &_materials[name];
		}
		else {
			return nullptr;
		}
	}

	Material* vk_renderer::create_material(VkPipeline pipeline, VkPipelineLayout layout, const std::string& name) {
		Material mat;
		mat.pipeline = pipeline;
		mat.pipelineLayout = layout;
		_materials[name] = mat;
		return &_materials[name];
	}

	void vk_renderer::load_meshes() {
		auto worker1 = std::async(std::launch::async, [&]() {
			Mesh::load_from_obj("assets/Interior/interior.asset", this);
		});

		auto worker2 = std::async(std::launch::async, [&]() {
			Mesh::load_from_obj("assets/Exterior/exterior.asset", this);
		});
	}

	void vk_renderer::upload_mesh(Mesh& mesh) {
		const size_t bufferSize = mesh._vertices.size() * sizeof(Vertex);

		// allocate Staging Buffer
		VkBufferCreateInfo stagingBufferInfo{};
		stagingBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		// total size in bytes
		stagingBufferInfo.size = bufferSize;
		stagingBufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

		// let vma know this buffer is gonna written by cpu and read by gpu
		VmaAllocationCreateInfo allocationInfo{};
		allocationInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		AllocatedBuffer stagingBuffer;

		VK_CHECK(vmaCreateBuffer(_allocator, &stagingBufferInfo, &allocationInfo, &stagingBuffer._buffer, &stagingBuffer._allocation, nullptr));

		// copy vertex data
		void* data;
		vmaMapMemory(_allocator, stagingBuffer._allocation, &data);

		memcpy(data, mesh._vertices.data(), bufferSize);

		vmaUnmapMemory(_allocator, stagingBuffer._allocation);

		// allocate Vertex Buffer
		VkBufferCreateInfo vertexBufferInfo{};
		vertexBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		// total size in bytes
		vertexBufferInfo.size = bufferSize;
		vertexBufferInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

		// let vma know this buffer is gonna written by cpu and read by gpu
		allocationInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		VK_CHECK(vmaCreateBuffer(_allocator, &vertexBufferInfo, &allocationInfo, &mesh._vertexBuffer._buffer, &mesh._vertexBuffer._allocation, nullptr));

		_deletionQueue.push_function([=]() {
			vmaDestroyBuffer(_allocator, mesh._vertexBuffer._buffer, mesh._vertexBuffer._allocation);
			});

		immediate_submit([=](VkCommandBuffer cmd) {
			VkBufferCopy copy;
			copy.srcOffset = 0;
			copy.dstOffset = 0;
			copy.size = bufferSize;
			vkCmdCopyBuffer(cmd, stagingBuffer._buffer, mesh._vertexBuffer._buffer, 1, &copy);
			});

		vmaDestroyBuffer(_allocator, stagingBuffer._buffer, stagingBuffer._allocation);
	}

	void vk_renderer::load_textures() {
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator("assets/San_Miguel/textures")) {
			auto worker = std::async(std::launch::async, [&]() {
				Texture texture;
				if (vk_util::load_image_from_file(this, dirEntry.path().string().c_str(), texture.Image)) {
					VkImageViewCreateInfo imageInfo = vk_info::ImageViewCreateInfo(texture.Image._image, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT);
					VK_CHECK(vkCreateImageView(_device, &imageInfo, nullptr, &texture.imageView));

					_textures[dirEntry.path().string()] = texture;

					_deletionQueue.push_function([=]() {
						vkDestroyImageView(_device, texture.imageView, nullptr);
						});
				}
			});
		};
	}

	void vk_renderer::immediate_submit(std::function<void(VkCommandBuffer cmd)>&& func) {
		VkCommandBufferAllocateInfo cmdAllocInfo{};
		cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdAllocInfo.commandPool = _uploadContext._commandPool;
		cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdAllocInfo.commandBufferCount = 1;

		VkCommandBuffer cmd{};
		VK_CHECK(vkAllocateCommandBuffers(_device, &cmdAllocInfo, &cmd));

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = 0; // optional
		beginInfo.pInheritanceInfo = nullptr; // optional

		VK_CHECK(vkBeginCommandBuffer(cmd, &beginInfo));

		func(cmd);

		VK_CHECK(vkEndCommandBuffer(cmd));

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmd;

		VK_CHECK(vkQueueSubmit(_graphicsQueue, 1, &submitInfo, _uploadContext._uploadFence));
		vkWaitForFences(_device, 1, &_uploadContext._uploadFence, VK_TRUE, 1000000000);
		vkResetFences(_device, 1, &_uploadContext._uploadFence);

		vkResetCommandPool(_device, _uploadContext._commandPool, NULL);
	}

	SwapChainSupportDetails vk_renderer::querySwapChainSupport(const VkPhysicalDevice& device) {
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

	VkSurfaceFormatKHR vk_renderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
		for (const auto& availableFormat : availableFormats) {
			if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return availableFormat;
			}
		}
		return availableFormats[0];
	}

	VkPresentModeKHR vk_renderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
		for (const auto& availablePresentMode : availablePresentModes) {
			if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) { // VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR
				return availablePresentMode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D vk_renderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
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

	VKAPI_ATTR VkBool32 VKAPI_CALL vk_renderer::debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
		std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

		return VK_FALSE;
	}

}