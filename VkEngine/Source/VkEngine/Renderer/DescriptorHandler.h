#pragma once

#include <vulkan/vulkan.h>
#include <span>
#include <vector>

namespace VkEngine
{

    class DescriptorHandler
    {
    public:

        struct DescriptorSetBuilder
        {
        public:

            template <typename T>
            void pushUniformBuffer(VkShaderStageFlags stage)
            {
                buffers.push_back(BufferHandler::createBuffer(sizeof(T), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU));
                bindings.push_back(createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, stage, bindings.size()));
                types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
            }

            template <typename T>
            void pushStorageBuffer(VkShaderStageFlags stage)
            {
                buffers.push_back(BufferHandler::createBuffer(sizeof(T), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY));
                bindings.push_back(createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, stage, bindings.size()));
                types.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER);
            }

            template <typename T>
            void pushDynamicUniformBuffer(VkShaderStageFlags stage)
            {
                buffers.push_back(BufferHandler::createBuffer(FRAME_OVERLAP.get() * sizeof(T), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VMA_MEMORY_USAGE_CPU_TO_GPU));
                bindings.push_back(createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, stage, bindings.size()));
                types.push_back(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC);
            }

            template <typename T>
            void pushDynamicStorageBuffer(VkShaderStageFlags stage)
            {
                buffers.push_back(BufferHandler::createBuffer(FRAME_OVERLAP.get() * sizeof(T), VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VMA_MEMORY_USAGE_GPU_ONLY));
                bindings.push_back(createDescriptorSetLayoutBinding(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, stage, bindings.size()));
                types.push_back(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);
            }

            void build(DeviceHandler* deviceHandle);

            std::vector<BufferID> buffers;

        private:

            VkDescriptorSetLayout layout;
            VkDescriptorSet descriptorSet;

            std::vector<VkDescriptorSetLayoutBinding> bindings;
            std::vector<VkDescriptorType> types;
        };

        DescriptorHandler(DeviceHandler* deviceHandle);
        ~DescriptorHandler();

        VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlag, uint32_t binding);
        void createDescriptorSetLayout(const std::span<VkDescriptorSetLayoutBinding>& bindings, VkDevice device, VkDescriptorSetLayout layout);
        void allocateDescriptorSets();

    private:
        
        VkDescriptorPool _descriptorPool;

        std::vector<DescriptorSetBuilder> _descriptorSets;

        BufferID _indirectBuffer;
    };
}