#pragma once

#include <vulkan/vulkan.h>
#include <span>
#include <vector>

#include "vk_mem_alloc.h"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"

namespace VkEngine
{

    class DeviceHandler;

    struct GPUCameraData
	{
		glm::mat4 view;
		glm::mat4 projection;
		glm::mat4 viewproj;
	};

	struct GPUSceneData
	{
		glm::vec4 fogColor; // w is for exponent
		glm::vec4 fogDistances; // x for min, y for max, zw unused
		glm::vec4 ambientColor;
		glm::vec4 sunlightDirection; // w for sun power
		glm::vec4 sunlightColor;
	};

    struct GPUObjectData
	{
		glm::mat4 modelMatrix;
	};

    constexpr uint16_t MAXOBJECTS = 65535;

    size_t padUniformBufferSize(size_t originalSize, VkPhysicalDeviceProperties deviceProperties);

    class DescriptorHandler
    {
    public:

        friend struct DescriptorSetBuilder;

        DescriptorHandler(DeviceHandler* deviceHandle);
        ~DescriptorHandler() {};

        inline std::vector<VkDescriptorSetLayout> getLayouts() { return _descriptorSetLayouts; };

        VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlag, uint32_t binding);
        void createDescriptorSetLayout(const std::span<VkDescriptorSetLayoutBinding>& bindings, VkDevice device, VkDescriptorSetLayout layout);
        // void allocateDescriptorSets();

    private:

        struct DescriptorSetBuilder
        {
        public:

            template <typename T>
            void pushUniformBuffer(VkShaderStageFlags stage)
            {
                buffers.push_back(BufferHandler::createBuffer(padUniformBufferSize(sizeof(T)), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU));
                bindings.push_back(createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage, bindings.size()));
                types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            }

            template <typename T>
            void pushStorageBuffer(VkShaderStageFlags stage)
            {
                buffers.push_back(BufferHandler::createBuffer(MAXOBJECTS * sizeof(T), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY));
                bindings.push_back(createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, stage, bindings.size()));
                types.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            }

            template <typename T>
            void pushDynamicUniformBuffer(VkShaderStageFlags stage)
            {
                buffers.push_back(BufferHandler::createBuffer(FRAME_OVERLAP.get() * padUniformBufferSize(sizeof(T)), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU));
                bindings.push_back(createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, stage, bindings.size()));
                types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
            }

            template <typename T>
            void pushDynamicStorageBuffer(VkShaderStageFlags stage)
            {
                buffers.push_back(BufferHandler::createBuffer(FRAME_OVERLAP.get() * sizeof(T) * MAXOBJECTS, VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY));
                bindings.push_back(createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, stage, bindings.size()));
                types.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
            }

            void build(DeviceHandler* deviceHandle, DescriptorHandler* descriptorHandler);

            std::vector<BufferID> buffers;

        private:

            VkDescriptorSet descriptorSet;
            VkDescriptorSetLayout layout;

            std::vector<VkDescriptorSetLayoutBinding> bindings;
            std::vector<VkDescriptorType> types;
        };
        
        VkDescriptorPool _descriptorPool;

        std::vector<VkDescriptorSet> _descriptorSets;
        std::vector<VkDescriptorSetLayout> _descriptorSetLayouts;

        // BufferID _indirectBuffer;
    };
}