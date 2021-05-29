#pragma once
#include "vk_type.h"

namespace vk_info {
	VkDebugUtilsMessengerCreateInfoEXT DebugMessengerCreateInfo(PFN_vkDebugUtilsMessengerCallbackEXT debugCallback);

	VkDeviceQueueCreateInfo DeviceQueueCreateInfo(uint32_t queueFamily, float& queuePriority);

	VkDeviceCreateInfo DeviceCreateInfo(const std::vector<VkDeviceQueueCreateInfo>& queueCreateInfos, VkPhysicalDeviceFeatures& deviceFeatures, const std::vector<const char*>& deviceExtensions);

	VkSwapchainCreateInfoKHR SwapChainCreateInfo(VkSurfaceKHR surface, uint32_t image_count, const SwapChainSupportDetails& swapChainSupport, VkSurfaceFormatKHR surfaceFormat, VkPresentModeKHR presentMode, VkExtent2D extent, QueueFamilyIndices indices);

	VkImageViewCreateInfo ImageViewCreateInfo(VkImage image, VkFormat format, VkImageAspectFlags aspectMask);

	VkImageCreateInfo ImageCreateInfo(VkFormat format, VkImageUsageFlags usageFlags, VkExtent3D extent); 

	VkDescriptorSetAllocateInfo DescriptorSetAllocateInfo(VkDescriptorPool pool, VkDescriptorSetLayout& layout);

	VkDescriptorImageInfo DescriptorImageInfo(VkSampler sampler, VkImageView imageView, VkImageLayout layout);

	VkDescriptorSetLayoutBinding DescriptorSetLayoutBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlag, uint32_t binding);

	VkWriteDescriptorSet WriteDescriptorSetBuffer(VkDescriptorType descriptorType, VkDescriptorSet dstSet, VkDescriptorBufferInfo* bufferInfo, uint32_t binding);

	VkWriteDescriptorSet WriteDescriptorSetImage(VkDescriptorType descriptorType, VkDescriptorSet dstSet, VkDescriptorImageInfo* bufferInfo, uint32_t binding);

	VkRenderPassCreateInfo RenderPassCreateInfo(VkAttachmentDescription* attachments, VkSubpassDescription& subpass, VkSubpassDependency& dependency);

	VkPipelineLayoutCreateInfo PipelineLayoutCreateInfo();

	VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo(const std::vector<VkVertexInputBindingDescription>& bindings, const std::vector<VkVertexInputAttributeDescription>& attributes);

	VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo(VkPrimitiveTopology topology);

	VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo(VkPolygonMode pMode);

	VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo();

	VkPipelineColorBlendAttachmentState ColorBlendAttachmentState();

	VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo(VkPipelineColorBlendAttachmentState& colorBlendAttachment);

	VkFramebufferCreateInfo FramebufferCreateInfo(VkRenderPass renderpass, VkExtent2D extent);

	VkPipelineDepthStencilStateCreateInfo PipelineDepthStencilStateCreateInfo (bool bDepthTest, bool bDepthWrite, VkCompareOp compareOp);

	VkSamplerCreateInfo SamplerCreateInfo(VkFilter filter, VkSamplerAddressMode addrMode);
}