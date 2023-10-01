#ifndef VULKANUNIFORMBUFFER_H
#define VULKANUNIFORMBUFFER_H

#include "VulkanHeader.h"

#include <vector>

namespace vkc
{
    template <class T>
    class UniformBuffer
    {
    public:
        UniformBuffer(uint32_t count = 1);
        UniformBuffer(const UniformBuffer&) = delete;
        UniformBuffer& operator=(const UniformBuffer&) = delete;

        ~UniformBuffer();

        void Update(const T* data, uint32_t index = 0);

    public:
        // Vulkan supports several frames in flight,
        // so we need multiple of these boys - one per frame, to avoid data race

        std::vector<VkBuffer> Buffers;
        std::vector<VkDeviceMemory> Memory;

        // VRAM mapping on the CPU to update data in the VkBuffer via memcpy
        std::vector<void*> HostMappings;
    };


}
#endif //VULKANUNIFORMBUFFER_H
