#pragma once
#include "vk_types.h"

namespace vk_init {
	VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);

	VkDeviceQueueCreateInfo DeviceQueueCreateInfo(uint32_t queueFamily, float queuePriority);

	VkDeviceCreateInfo DeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, const VkPhysicalDeviceFeatures& deviceFeatures, const std::vector<const char*>& deviceExtensions);

	VkSwapchainCreateInfoKHR SwapChainCreateInfo(const VkSurfaceKHR& surface, uint32_t image_count, const SwapChainSupportDetails& swapChainSupport, const VkSurfaceFormatKHR& surfaceFormat, const VkPresentModeKHR& presentMode, const VkExtent2D& extent, const QueueFamilyIndices& indices);

	VkImageViewCreateInfo ImageViewCreateInfo(const VkImage& image, const VkFormat& format, const VkImageAspectFlags& aspectMask);

	VkImageCreateInfo ImageCreateInfo(const VkFormat& format, const VkImageUsageFlags& usageFlags, const VkExtent3D& extent); 

	VkRenderPassCreateInfo RenderPassCreateInfo(const VkAttachmentDescription* attachments, const VkSubpassDescription& subpass, const VkSubpassDependency& dependency);

	VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();

	VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo(const std::vector<VkVertexInputBindingDescription>& bindings, const std::vector<VkVertexInputAttributeDescription>& attributes);

	VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo(const VkPrimitiveTopology& topology);

	VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(const VkPolygonMode& pMode);

	VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo();

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState();

	VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo(const VkPipelineColorBlendAttachmentState& colorBlendAttachment);

	VkFramebufferCreateInfo FramebufferCreateInfo(const VkRenderPass& renderpass, const VkExtent2D& extent);

	VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo (bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);
}