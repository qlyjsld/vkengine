#include "VkEngine/Renderer/PipelineHandler.h"
#include "VkEngine/Renderer/DeletionQueue.h"

namespace VkEngine
{

	PipelineHandler::PipelineHandler(DescriptorHandler* descriptorHandle)
	{
		PipelineBuilder graphicPipelineInfo = buildGraphicPipeline("shaders/vert.spv", "shaders/frag.spv", descriptorHandle->getLayouts());
	}

	PipelineBuilder PipelineHandler::buildGraphicPipeline(const std::string& vertexShader, const std::string& fragmentShader, const std::vector<VkDescriptorSetLayout>& layouts)
	{
		PipelineBuilder pipelineInfo{};

		auto vertShaderCode = readfile(vertexShader);
		auto fragShaderCode = readfile(fragmentShader);
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

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0; // Optional
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

		// hook the global set layout
		pipelineLayoutInfo.setLayoutCount = layouts.size();
		pipelineLayoutInfo.pSetLayouts = layouts.data();

		VkPipelineLayout pipelineLayout{};

		VK_CHECK(vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &pipelineLayout));

		_pipelineLayouts.push_back(std::move(pipelineLayout));

		DeletionQueue::push_function([=]()
		{
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

		pipelineInfo._shaderStages = shaderStages;
		pipelineInfo._vertexInputInfo = vertexInputInfo;
		pipelineInfo._inputAssembly = inputAssembly;
		pipelineInfo._viewportState = viewportState;
		pipelineInfo._rasterizer = rasterizer;
		pipelineInfo._multisampling = multisampling;
		pipelineInfo._depthStencil = DepthStencilState;
		pipelineInfo._colorBlendAttachment = colorBlendAttachment;
		pipelineInfo._dynamicState = dynamicState;
		pipelineInfo._pipelineLayout = _pipelineLayout;

		return pipelineInfo;
	}

	
	VkPipeline PipelineHandler::PipelineBuilder::build_pipeline(VkDevice device, VkRenderPass pass)
	{
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

}