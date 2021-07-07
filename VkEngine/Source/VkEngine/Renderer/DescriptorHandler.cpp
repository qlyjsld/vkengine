#include "VkEngine/Renderer/DescriptorHandler.h"

namespace VkEngine
{

    void DescriptorHandler::init()
    {
        // create indirect buffer
		_indirectBuffer = create_buffer(FRAME_OVERLAP * sizeof(VkDrawIndirectCommand), VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		_deletionQueue.push_function([=]()
		{
			vmaDestroyBuffer(_allocator, _indirectBuffer._buffer, _indirectBuffer._allocation);
		});

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

		VK_CHECK(vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_descriptorPool));

		_deletionQueue.push_function([&]()
		{
			vkDestroyDescriptorPool(_device, _descriptorPool, nullptr);
		});

		size_t sceneParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUSceneData));
		_sceneParametersBuffer = create_buffer(sceneParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		_deletionQueue.push_function([=]()
		{
			vmaDestroyBuffer(_allocator, _sceneParametersBuffer._buffer, _sceneParametersBuffer._allocation);
		});

		size_t cameraParamBufferSize = FRAME_OVERLAP * pad_uniform_buffer_size(sizeof(GPUCameraData));
		_cameraParametersBuffer = create_buffer(cameraParamBufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

		_deletionQueue.push_function([=]()
		{
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

		_deletionQueue.push_function([&]()
		{
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

		_deletionQueue.push_function([&]()
		{
			vkDestroyDescriptorSetLayout(_device, _objectSetLayout, nullptr);
		});

		for (int i = 0; i < FRAME_OVERLAP; i++)
		{
			const int MAX_OBJECTS = 32767;
			_frames[i]._objectBuffer = create_buffer(sizeof(GPUObjectData) * MAX_OBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU);

			_deletionQueue.push_function([=]()
			{
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

		_deletionQueue.push_function([&]()
		{
			vkDestroyDescriptorSetLayout(_device, _textureSetLayout, nullptr);
		});
    }
}