#include "VkEngine/Renderer/DescriptorHandler.h"
#include "VkEngine/Renderer/BufferHandler.h"
#include "VkEngine/Renderer/DeletionQueue.h"
#include "VkEngine/Core/ConsoleVariableSystem.h"

#include <vulkan/vulkan.h>
#include <array>

AutoInt FRAME_OVERLAP(2, "FRAME_OVERLAP", "FRAME_OVERLAP", ConsoleVariableFlag::NONE);

namespace VkEngine
{

	void DescriptorHandler::DescriptorSetBuilder::build(DeviceHandler* deviceHandle)
	{
		if (!buffers.empty() && !bindings.empty())
		{
			VkDescriptorSetLayoutCreateInfo setInfo{};
			setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			setInfo.pNext = nullptr;

			setInfo.bindingCount = bindings.size();
			setInfo.flags = 0;
			setInfo.pBindings = bindings.data();

			VK_CHECK(vkCreateDescriptorSetLayout(deviceHandle->getDevice(), &setInfo, nullptr, &layout));
			_descriptorSetLayouts.push_back(layout);

			DeletionQueue::push_function([=]()
			{
				vkDestroyDescriptorSetLayout(deviceHandle->getDevice(), layout, nullptr);
			});

			VkDescriptorSetAllocateInfo allocInfo{};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.pNext = nullptr;

			allocInfo.descriptorPool = _descriptorPool;
			allocInfo.descriptorSetCount = 1;
			allocInfo.pSetLayouts = &layout;

			vkAllocateDescriptorSets(deviceHandle->getDevice(), &allocInfo, &descriptorSet);
			_descriptorSets.push_back(descriptorSet);

			for (uint32_t i = 0; i < buffers.size(); i++)
			{
				VkDescriptorBufferInfo bufferInfo{};
				bufferInfo.buffer = BufferHandler::getBuffer(buffers[i]);
				bufferInfo.offset = 0;
				bufferInfo.range = sizeof(BufferHandler::getBufferSize(buffers[i]));

				VkWriteDescriptorSet writeSet{};
				writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeSet.pNext = nullptr;

				writeSet.dstBinding = i;
				writeSet.dstSet = descriptorSet;
				writeSet.descriptorCount = 1;
				writeSet.descriptorType = types[i];
				writeSet.pBufferInfo = bufferInfo;

				VkWriteDescriptorSet writeOperation = { writeSet };
				vkUpdateDescriptorSets(deviceHandle->getDevice(), 1, writeOperation, 0, nullptr);
			}
		}
	}

    void DescriptorHandler::DescriptorHandler(DeviceHandler* deviceHandle)
    {
        // create indirect buffer
		// _indirectBuffer = BufferHandler::createBuffer(FRAME_OVERLAP.get() * sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		// create a descriptor pool that will hold 10 uniform buffers
		std::vector<VkDescriptorPoolSize> sizes =
		{
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

		VK_CHECK(vkCreateDescriptorPool(deviceHandle->getDevice(), &poolInfo, nullptr, &_descriptorPool));

		DeletionQueue::push_function([=]()
		{
			vkDestroyDescriptorPool(deviceHandle->getDevice(), _descriptorPool, nullptr);
		});

		DescriptorSetBuilder descriporSet1;
        descriporSet1.pushDynamicUniformBuffer<GPUCameraData>(VK_SHADER_STAGE_VERTEX_BIT);
        descriporSet1.pushDynamicUniformBuffer<GPUSceneData>(VK_SHADER_STAGE_FRAGMENT_BIT);

        DescriptorSetBuilder descriporSet2;
        descriporSet2.pushDynamicStorageBuffer<GPUObjectData>(VK_SHADER_STAGE_VERTEX_BIT);

        descriporSet1.build(deviceHandle);
		descriporSet2.build(deviceHandle);
    }

	VkDescriptorSetLayoutBinding DescriptorHandler::createDescriptorSetLayoutBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlag, uint32_t binding)
	{
		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = binding;
		layoutBinding.descriptorCount = 1;
		layoutBinding.descriptorType = descriptorType;
		layoutBinding.stageFlags = stageFlag;

		return layoutBinding;
	}

	void DescriptorHandler::createDescriptorSetLayout(const std::span<VkDescriptorSetLayoutBinding>& bindings, VkDevice device, VkDescriptorSetLayout layout)
	{
		VkDescriptorSetLayoutCreateInfo setInfo{};
		setInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		setInfo.pNext = nullptr;

		setInfo.bindingCount = bindings.size();
		setInfo.flags = 0;
		setInfo.pBindings = bindings;

		VK_CHECK(vkCreateDescriptorSetLayout(device, &setInfo, nullptr, &layout));

		DeletionQueue::push_function([=]()
		{
			vkDestroyDescriptorSetLayout(device, layout, nullptr);
		});
	}

	size_t padUniformBufferSize(size_t originalSize)
	{
		size_t minUboAlignment = _deviceProperties.limits.minUniformBufferOffsetAlignment;
		size_t alignedSize = originalSize;
		if (minUboAlignment > 0)
		{
			alignedSize = (alignedSize + minUboAlignment - 1) & ~(minUboAlignment - 1);
		}
		return alignedSize;
	}
}