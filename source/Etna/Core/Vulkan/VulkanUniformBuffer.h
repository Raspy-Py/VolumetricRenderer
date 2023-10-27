#ifndef VULKANUNIFORMBUFFER_H
#define VULKANUNIFORMBUFFER_H

#include "VulkanCore.h"
#include "VulkanContext.h"
#include "VulkanBindableInterface.h"

#include <vector>
#include <cstring>

namespace vkc
{
    template <class T>
    class UniformBuffer : public DescriptiveBufferInterface
    {
    public:
        UniformBuffer(uint32_t count = 1);
        UniformBuffer(const UniformBuffer&) = delete;
        UniformBuffer& operator=(const UniformBuffer&) = delete;

        ~UniformBuffer() = default;

        void Update(const T* data, uint32_t index = 0);

    public:
        // From DescriptiveBufferInterface

        VkDescriptorSetLayout CreateDescriptorSetLayout(uint32_t binding, ShaderStage shaderStage) override;
        std::vector<VkDescriptorSet> CreateDescriptorSets(VkDescriptorSetLayout layout, const vkc::DescriptorPool& pool) override;

    public:
        // Vulkan supports several frames in flight,
        // so we need multiple of these boys - one per frame, to avoid data race

        std::vector<VkBuffer> Buffers;
        std::vector<VkDeviceMemory> Memory;

        // VRAM mapping on the CPU to update data in the VkBuffer via memcpy
        std::vector<void*> HostMappings;
    };

    template <class T>
    UniformBuffer<T>::UniformBuffer(uint32_t count)
    {
        uint32_t bufferSize = sizeof(T);
        Buffers.resize(count);
        Memory.resize(count);
        HostMappings.resize(count);
        for (uint32_t i = 0; i < count; i++)
        {
            CreateBuffer(
                bufferSize,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                Buffers[i],
                Memory[i]
            );

            vkMapMemory(Context::GetDevice(), Memory[i], 0, bufferSize, 0, &HostMappings[i]);
        }
    }

    template <class T>
    void UniformBuffer<T>::Update(const T *data, uint32_t index)
    {
        memcpy(HostMappings[index], data, sizeof(T));
    }

    template <class T>
    VkDescriptorSetLayout UniformBuffer<T>::CreateDescriptorSetLayout(uint32_t binding, ShaderStage shaderStage)
    {
        return vkc::CreateDescriptorSetLayout(binding, vkc::DescriptorType::UniformBuffer, shaderStage);
    }

    template <class T>
    std::vector<VkDescriptorSet> UniformBuffer<T>::CreateDescriptorSets(VkDescriptorSetLayout layout, const vkc::DescriptorPool& pool)
    {
        std::vector<VkDescriptorSet> sets(Buffers.size());
        vkc::AllocateDescriptorSets(layout, pool.Handle, sets.data(), sets.size());
        vkc::UpdateBufferDescriptorSets<T>(Buffers.data(), sets.data(), sets.size());

        return sets;
    }
}
#endif //VULKANUNIFORMBUFFER_H
