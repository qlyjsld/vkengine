#pragma once
#include <vulkan/vulkan.h>

namespace VkEngine
{

	class DescriptorHandler;

	class PipelineHandler
	{
	public:

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

		PipelineHandler(DeviceHandler* deviceHandle, DescriptorHandler* descriptorHandle, RenderPassHandler* renderPassHandle);
		~PipelineHandler() {};

	private:

		PipelineBuilder buildGraphicPipeline(const std::string& vertexShader, const std::string& fragmentShader, const VkDescriptorSetLayout* layouts);

		std::vector<VkPipeline> _pipelines;
		std::vector<VkPipelineLayout> _pipelineLayouts;
	};
}