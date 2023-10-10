#include "VulkanUniformBuffer.h"

#include "VulkanCore.h"
#include "VulkanContext.h"

#include <cstring>

namespace vkc
{
    template <class T>
    UniformBuffer<T>::UniformBuffer(uint32_t count)
    {
        uint32_t bufferSize = sizeof(T);

        for (uint32_t i = 0; i < count; ++i)
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
    std::vector<VkDescriptorSet> UniformBuffer<T>::CreateDescriptorSets(VkDescriptorSetLayout layout, vkc::DescriptorPool pool)
    {
        std::vector<VkDescriptorSet> sets(Buffers.size());
        vkc::AllocateDescriptorSets(layout, pool.Handle, sets.data(), sets.size());
        vkc::UpdateBufferDescriptorSets<T>(Buffers.data(), sets.data(), sets.size());

        return sets;
    }
}
