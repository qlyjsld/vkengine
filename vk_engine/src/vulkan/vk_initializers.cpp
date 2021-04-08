#include <utility>
#include "vk_initializers.h"

VkDebugUtilsMessengerCreateInfoEXT vk_init::DebugMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback) {
	VkDebugUtilsMessengerCreateInfoEXT createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = debugCallback;
	createInfo.pUserData = nullptr;

	return (createInfo);
}

VkDeviceQueueCreateInfo vk_init::DeviceQueueCreateInfo(uint32_t queueFamily, float& queuePriority) {
	VkDeviceQueueCreateInfo queueCreateInfo{};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = queueFamily;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;

	return (queueCreateInfo);
}

VkDeviceCreateInfo vk_init::DeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, VkPhysicalDeviceFeatures& deviceFeatures, const std::vector<const char*>& deviceExtensions) {
	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	deviceCreateInfo.enabledLayerCount = 0;

	return (deviceCreateInfo);
}

VkSwapchainCreateInfoKHR vk_init::SwapChainCreateInfo(VkSurfaceKHR surface, uint32_t image_count, const SwapChainSupportDetails& swapChainSupport, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkExtent2D extent, QueueFamilyIndices indices) {
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;

	createInfo.minImageCount = image_count;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = { indices.graphicFamily.value(), indices.presentFamily.value() };

	if (indices.graphicFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	return createInfo;
}

VkImageViewCreateInfo vk_init::ImageViewCreateInfo(VkImage image, VkFormat format, VkImageAspectFlags aspectMask) {
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.image = image;

	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;

	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

	createInfo.subresourceRange.aspectMask = aspectMask;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	return createInfo;
}

VkImageCreateInfo vk_init::ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent) {
	VkImageCreateInfo info{};
	info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	info.pNext = nullptr;

	info.imageType = VK_IMAGE_TYPE_2D;

	info.format = format;
	info.extent = extent;

	info.mipLevels = 1;
	info.arrayLayers = 1;
	info.samples = VK_SAMPLE_COUNT_1_BIT;
	info.tiling = VK_IMAGE_TILING_OPTIMAL;
	info.usage = usageFlags;

	return info;
}

VkRenderPassCreateInfo vk_init::RenderPassCreateInfo(VkAttachmentDescription* attachments, VkSubpassDescription& subpass, VkSubpassDependency& dependency) {
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = 2;
	renderPassInfo.pAttachments = &attachments[0];
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	return renderPassInfo;
}

VkPipelineLayoutCreateInfo vk_init::PipelineLayoutCreateInfo() {
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 0; // Optional
	pipelineLayoutInfo.pSetLayouts = nullptr;
	pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

	return pipelineLayoutInfo;
}

VkPipelineVertexInputStateCreateInfo vk_init::VertexInputStateCreateInfo(const std::vector<VkVertexInputBindingDescription>& bindings, const std::vector<VkVertexInputAttributeDescription>& attributes){
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount = bindings.size();
	vertexInputInfo.pVertexBindingDescriptions = bindings.data(); // Optional
	vertexInputInfo.vertexAttributeDescriptionCount = attributes.size();
	vertexInputInfo.pVertexAttributeDescriptions = attributes.data(); // Optional

	return vertexInputInfo;
}

VkPipelineInputAssemblyStateCreateInfo vk_init::InputAssemblyStateCreateInfo(VkPrimitiveTopology topology) {
	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = topology;
	inputAssembly.primitiveRestartEnable = VK_FALSE;

	return inputAssembly;
}

VkPipelineRasterizationStateCreateInfo vk_init::RasterizationStateCreateInfo(VkPolygonMode pMode) {
	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = pMode;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;
	rasterizer.depthBiasConstantFactor = 0.0f; // Optional
	rasterizer.depthBiasClamp = 0.0f; // Optional
	rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

	return rasterizer;
}

VkPipelineMultisampleStateCreateInfo vk_init::MultisampleStateCreateInfo() {
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampling.minSampleShading = 1.0f; // Optional
	multisampling.pSampleMask = nullptr; // Optional
	multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
	multisampling.alphaToOneEnable = VK_FALSE; // Optional

	return multisampling;
}

VkPipelineColorBlendAttachmentState vk_init::ColorBlendAttachmentState() {
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;
	colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
	colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
	colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
	colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

	return colorBlendAttachment;
}

VkPipelineColorBlendStateCreateInfo vk_init::ColorBlendStateCreateInfo(VkPipelineColorBlendAttachmentState& colorBlendAttachment) {
	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	colorBlending.blendConstants[0] = 0.0f; // Optional
	colorBlending.blendConstants[1] = 0.0f; // Optional
	colorBlending.blendConstants[2] = 0.0f; // Optional
	colorBlending.blendConstants[3] = 0.0f; // Optional

	return colorBlending;
}

VkFramebufferCreateInfo vk_init::FramebufferCreateInfo(VkRenderPass renderpass, VkExtent2D extent) {
	VkFramebufferCreateInfo framebufferInfo{};
	framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferInfo.renderPass = renderpass;
	framebufferInfo.attachmentCount = 1;
	framebufferInfo.width = extent.width;
	framebufferInfo.height = extent.height;
	framebufferInfo.layers = 1;

	return framebufferInfo;
}

VkPipelineDepthStencilStateCreateInfo vk_init::PipelineDepthStencilStateCreateInfo (bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp) {
	VkPipelineDepthStencilStateCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	createInfo.pNext = nullptr;

	createInfo.depthTestEnable = bDepthTest ? VK_TRUE : VK_FALSE;
	createInfo.depthWriteEnable = bDepthWrite ? VK_TRUE : VK_FALSE;
	createInfo.depthCompareOp = bDepthTest ? compareOp : VK_COMPARE_OP_ALWAYS;
	createInfo.depthBoundsTestEnable = VK_FALSE;
	createInfo.minDepthBounds = 0.0f; // Optional
	createInfo.maxDepthBounds = 1.0f; // Optional
	createInfo.stencilTestEnable = VK_FALSE;

	return createInfo;
}