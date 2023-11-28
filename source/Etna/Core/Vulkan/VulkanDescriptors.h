#ifndef VULKANDESCRIPTORS_H
#define VULKANDESCRIPTORS_H

#include "VulkanCore.h"
#include <memory>
#include <map>

namespace vkc
{
    class DescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder() = default;
            ~Builder() = default;

            Builder& AddBinding(
                uint32_t binding,
                vkc::DescriptorType type,
                vkc::ShaderStage shaderStage,
                uint32_t count = 1);
            std::unique_ptr<DescriptorSetLayout> Build();
        private:
            std::map<uint32_t, VkDescriptorSetLayoutBinding> Bindings;
        };
    public:
        DescriptorSetLayout() = default;
        ~DescriptorSetLayout() = default;

        VkDescriptorSetLayout Handle;
    private:
        std::map<uint32_t, VkDescriptorSetLayoutBinding> Bindings;

        friend class DescriptorSetPool;
        friend class DescriptorSetWriter;
    };

    class DescriptorSetPool
    {
    public:
        DescriptorSetPool(const DescriptorSetLayout& layout, uint32_t count = 1);
        ~DescriptorSetPool();

        VkDescriptorSet AllocateSet(const DescriptorSetLayout& layout);

    private:
        VkDescriptorPool Handle;
    };

    class DescriptorSetWriter
    {
    public:
        DescriptorSetWriter(vkc::DescriptorSetLayout& layout, vkc::DescriptorSetPool& pool);

        DescriptorSetWriter& WriteBuffer(uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
        DescriptorSetWriter& WriteImage(uint32_t binding, VkImageView view, VkSampler sampler);

        VkDescriptorSet Write();

    private:
        std::vector<VkDescriptorBufferInfo> BufferInfos;
        std::vector<VkDescriptorImageInfo> ImageInfos;
        std::vector<VkWriteDescriptorSet> WriteSets;
        VkDescriptorSet Set;

        vkc::DescriptorSetLayout& Layout;
        vkc::DescriptorSetPool& Pool;
    };
}

/*
 * typedef struct VkDescriptorBufferInfo {
    VkBuffer        buffer;
    VkDeviceSize    offset;
    VkDeviceSize    range;
} VkDescriptorBufferInfo;
 * typedef struct VkDescriptorImageInfo {
    VkSampler        sampler;
    VkImageView      imageView;
    VkImageLayout    imageLayout;
} VkDescriptorImageInfo;
 * */

#endif //VULKANDESCRIPTORS_H
