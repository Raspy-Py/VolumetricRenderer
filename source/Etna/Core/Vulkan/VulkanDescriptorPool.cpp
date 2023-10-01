#include "VulkanDescriptorPool.h"

#include "Etna/Core/Utils.h"
#include "VulkanContext.h"

namespace  vkc
{
    VkDescriptorType ToVulkanNativeType(vkc::DescriptorType type)
    {
        return static_cast<VkDescriptorType>(type);
    }

    DescriptorPool::DescriptorPool(vkc::DescriptorType type, uint32_t maxSets)
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = ToVulkanNativeType(type);
        poolSize.descriptorCount = maxSets;

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = maxSets;

        if (vkCreateDescriptorPool(Context::GetDevice(), &poolInfo, Context::GetAllocator(), &Handle) != VK_SUCCESS)
        {
            Error("Failed to create descriptor pool.");
        }
    }

    DescriptorPool::~DescriptorPool()
    {
        vkDestroyDescriptorPool(Context::GetDevice(), Handle, Context::GetAllocator());
    }
}
