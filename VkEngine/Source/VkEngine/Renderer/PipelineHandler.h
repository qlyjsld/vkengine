#pragma once
#include <vulkan/vulkan.h>

#include "VkEngine/Renderer/DescriptorHandler.h"

namespace VkEngine
{

	class PipelineHandler
	{
	public:

		PipelineHandler(DescriptorHandler* descriptorHandle);
		~PipelineHandler() {};

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
			VkPipeline build_pipeline(VkDevice device, VkRenderPass pass);
		};

	private:

		PipelineBuilder buildGraphicPipeline(const std::string& vertexShader, const std::string& fragmentShader, const VkDescriptorSetLayout* layouts);

		std::vector<VkPipeline> _pipelines;
		std::vector<VkPipelineLayout> _pipelineLayouts;
	};
}