#ifndef VULKANDESCRIPTORPOOL_H
#define VULKANDESCRIPTORPOOL_H

#include "VulkanHeader.h"

namespace vkc
{
    enum class DescriptorType : int
    {
        Sampler = 0,
        CombinedImageSampler = 1,
        SampledImage = 2,
        StorageImage = 3,
        UniformTexelBuffer = 4,
        StorageTexelBuffer = 5,
        UniformBuffer = 6,
        StorageBuffer = 7,
        UniformBufferDynamic = 8,
        StorageBufferDynamic = 9,
        InputAttachment = 10,
        InlineUniformBlock = 1000138000,
        AccelerationStructureKHR = 1000150000,
        AccelerationStructureNV = 1000165000,
        MutableValue = 1000351000,
        InlineUniformBlockEXT = InlineUniformBlock,
        MaxEnum = 0x7FFFFFFF
    };

    VkDescriptorType ToVulkanNativeType(vkc::DescriptorType type);

    class DescriptorPool
    {
    public:
        DescriptorPool(DescriptorType type, uint32_t maxSets);
        ~DescriptorPool();

    private:
        VkDescriptorPool Handle;
    };
}


#endif //VULKANDESCRIPTORPOOL_H
