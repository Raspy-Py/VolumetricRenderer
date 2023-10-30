#include "VulkanDescriptors.h"

#include "Etna/Core/Utils.h"

namespace vkc
{
    DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::AddBinding(
        uint32_t binding,
        vkc::DescriptorType type,
        vkc::ShaderStage shaderStage,
        uint32_t count)
    {
        if (Bindings.contains(binding))
        {
            Error("This binding location is already occupied: %i.", (int)binding);
        }

        Bindings.emplace(
            binding,
            VkDescriptorSetLayoutBinding{
                binding,
                static_cast<VkDescriptorType>(type),
                count,
                static_cast<VkShaderStageFlags>(shaderStage),
                nullptr
            }
        );

        return *this;
    }

    std::unique_ptr<DescriptorSetLayout> DescriptorSetLayout::Builder::Build()
    {
        std::vector<VkDescriptorSetLayoutBinding> bindingsData;
        for (auto& binding : Bindings)
        {
            bindingsData.push_back(binding.second);
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindingsData.size());
        layoutInfo.pBindings = bindingsData.data();

        auto layout = new DescriptorSetLayout;
        layout->Bindings = std::move(Bindings);
        if (vkCreateDescriptorSetLayout(Context::GetDevice(), &layoutInfo, nullptr, &layout->Handle) != VK_SUCCESS)
        {
            Error("Failed to create descriptor set layout.");
        }

        return std::unique_ptr<DescriptorSetLayout>(layout);
    }

    DescriptorSetPool::DescriptorSetPool(const DescriptorSetLayout& layout, uint32_t count)
    {
        uint32_t bindingsCount = layout.Bindings.size();
        std::vector<VkDescriptorPoolSize> poolSizes;
        for (auto& binding : layout.Bindings)
        {
            poolSizes.emplace_back(binding.second.descriptorType, count);
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = bindingsCount;
        poolInfo.pPoolSizes = poolSizes.data();
        poolInfo.maxSets = count;

        if (vkCreateDescriptorPool(Context::GetDevice(), &poolInfo, Context::GetAllocator(), &Handle) != VK_SUCCESS)
        {
            Error("Failed to create descriptor pool.");
        }
    }
    DescriptorSetPool::~DescriptorSetPool()
    {
        vkDestroyDescriptorPool(Context::GetDevice(), Handle, Context::GetAllocator());
    }

    VkDescriptorSet DescriptorSetPool::AllocateSet(const DescriptorSetLayout& layout)
    {
        VkDescriptorSet set;

        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = Handle;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &layout.Handle;

        VkResult result = vkAllocateDescriptorSets(Context::GetDevice(), &allocInfo, &set);
        if (result == VK_ERROR_OUT_OF_POOL_MEMORY)
        {
            Error("Out of memory.");
        }
        else
        {

        }

        return set;
    }

    DescriptorSetWriter::DescriptorSetWriter(DescriptorSetLayout &layout, DescriptorSetPool &pool)
        : Layout(layout)
        , Pool(pool)
    {
        Set = Pool.AllocateSet(Layout);
    }

    DescriptorSetWriter &DescriptorSetWriter::WriteBuffer(
        uint32_t binding,
        VkBuffer buffer,
        VkDeviceSize offset,
        VkDeviceSize range)
    {
        auto pLayoutBinding = Layout.Bindings.find(binding);
        if (pLayoutBinding == Layout.Bindings.end())
        {
            Error("This layout doesn't have binding to %i.", (int)binding);
        }

        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = buffer;
        bufferInfo.offset = offset;
        bufferInfo.range = range;
        BufferInfos.push_back(bufferInfo);

        VkWriteDescriptorSet writeSet{};
        writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSet.dstSet = Set;
        writeSet.dstBinding = binding;
        writeSet.dstArrayElement = 0;
        writeSet.descriptorType = pLayoutBinding->second.descriptorType;
        writeSet.descriptorCount = pLayoutBinding->second.descriptorCount;
        writeSet.pBufferInfo = &BufferInfos.back();
        WriteSets.push_back(writeSet);

        return *this;
    }

    DescriptorSetWriter& DescriptorSetWriter::WriteImage(
        uint32_t binding,
        VkImageView view,
        VkSampler sampler)
    {
        auto pLayoutBinding = Layout.Bindings.find(binding);
        if (pLayoutBinding == Layout.Bindings.end())
        {
            Error("This layout doesn't have binding to %i.", (int)binding);
        }

        VkDescriptorImageInfo imageInfo{};
        imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        imageInfo.imageView = view;
        imageInfo.sampler = sampler;
        ImageInfos.push_back(imageInfo);

        VkWriteDescriptorSet writeSet{};
        writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        writeSet.dstSet = Set;
        writeSet.dstBinding = binding;
        writeSet.dstArrayElement = 0;
        writeSet.descriptorType = pLayoutBinding->second.descriptorType;
        writeSet.descriptorCount = pLayoutBinding->second.descriptorCount;
        writeSet.pImageInfo = &ImageInfos.back();
        WriteSets.push_back(writeSet);

        return *this;
    }

    VkDescriptorSet DescriptorSetWriter::Write()
    {
        vkUpdateDescriptorSets(Context::GetDevice(), WriteSets.size(), WriteSets.data(), 0, nullptr);

        return Set;
    }
}