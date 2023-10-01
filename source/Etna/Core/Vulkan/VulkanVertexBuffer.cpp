#include "VulkanVertexBuffer.h"

#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace vkc
{
    template <typename V>
    VertexBuffer<V>::VertexBuffer(VkDeviceSize size)
        : Size(size * sizeof(V))
    {
        CreateBuffer(
            Size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            Buffer,
            Memory
        );
    }

    template <typename V>
    VertexBuffer<V>::VertexBuffer(VkCommandPool cmdPool, V* data, VkDeviceSize size)
        : Size(size * sizeof(V))
    {
        CreateBuffer(
            Size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            Buffer,
            Memory
        );

        Update(cmdPool, data);
    }

    template <typename V>
    void VertexBuffer<V>::Update(VkCommandPool cmdPool, V *data)
    {
        /*
         * Vertex buffer uses device local memory, which is not visible to the CPU,
         * so it can't be written from it. To put some data into this buffer, we create
         * special host visible staging buffer, fill it and then transfer it's contains
         * into the index buffer.
         */

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        CreateBuffer(
            Size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            stagingBuffer,
            stagingBufferMemory
        );

        FillBuffer(stagingBufferMemory, data, Size);
        CopyBuffer(cmdPool, stagingBuffer, Buffer, Size);

        vkDestroyBuffer(Context::GetDevice(), stagingBuffer, Context::GetAllocator());
        vkFreeMemory(Context::GetDevice(), stagingBufferMemory, Context::GetAllocator());
    }
}
