#ifndef VULKANINDEXBUFFER_H
#define VULKANINDEXBUFFER_H

#include "VulkanHeader.h"

namespace vkc
{
    class IndexBuffer
    {
    public:
        IndexBuffer(VkDeviceSize size);
        IndexBuffer(VkCommandPool cmdPool, uint16_t* data, VkDeviceSize size);
        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        ~IndexBuffer();

        void Update(VkCommandPool cmdPool, uint16_t* data);

    private:
        VkBuffer Buffer;
        VkDeviceMemory Memory;
        VkDeviceSize Size;
    };
}

#endif //VULKANINDEXBUFFER_H
