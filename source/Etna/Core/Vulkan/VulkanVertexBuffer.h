#ifndef VULKANVERTEXBUFFER_H
#define VULKANVERTEXBUFFER_H

#include "VulkanHeader.h"

namespace  vkc
{
    template <typename V>
    class VertexBuffer
    {
    public:
        VertexBuffer(VkDeviceSize size);
        VertexBuffer(VkCommandPool cmdPool, V* data, VkDeviceSize size);
        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        ~VertexBuffer();

        void Update(VkCommandPool cmdPool, V* data);

    private:
        VkBuffer Buffer;
        VkDeviceMemory Memory;
        VkDeviceSize Size;
    };
}


#endif //VULKANVERTEXBUFFER_H
