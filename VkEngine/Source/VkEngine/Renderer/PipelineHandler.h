#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace VkEngine
{

	class DeviceHandler;
	class DescriptorHandler;
	class RenderPassHandler;

	class PipelineHandler
	{
	public:

		PipelineHandler(DeviceHandler* deviceHandle, DescriptorHandler* descriptorHandle, RenderPassHandler* renderPassHandle);
		~PipelineHandler() {};

	private:

		class PipelineBuilder
		{
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
			VkPipeline buildPipeline(VkDevice device, VkRenderPass renderPass);
		};

		PipelineBuilder buildGraphicPipeline(const std::string& vertexShader, const std::string& fragmentShader, const std::vector<VkDescriptorSetLayout>& layouts, VkDevice device);

		std::vector<VkPipeline> _pipelines;
		std::vector<VkPipelineLayout> _pipelineLayouts;
	};
}