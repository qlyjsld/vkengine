#pragma once

#include <vulkan/vulkan.h>
#include <span>

namespace VkEngine
{

    class DescriptorHandler
    {
    public:

        DescriptorHandler(DeviceHandler* deviceHandle)
        {
            init(deviceHandle);
        }

        ~DescriptorHandler();

        VkDescriptorPool _descriptorPool;
        VkDescriptorSetLayout set0Layout;

        BufferID _indirectBuffer;
        BufferID _sceneParametersBuffer;
        BufferID _cameraParametersBuffer;

        VkDescriptorSetLayoutBinding createDescriptorSetLayoutBinding(VkDescriptorType descriptorType, VkShaderStageFlags stageFlag, uint32_t binding);
        void createDescriptorSetLayout(const std::span<VkDescriptorSetLayoutBinding>& bindings, VkDevice device, VkDescriptorSetLayout layout);
        void allocateDescriptorSets();

        void init(DeviceHandler* deviceHandle);
        void release();
    };
}