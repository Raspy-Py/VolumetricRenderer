#ifndef VULKANVERTEXBUFFER_H
#define VULKANVERTEXBUFFER_H

#include "VulkanCore.h"
#include "VulkanContext.h"
#include "VulkanBindableInterface.h"

namespace  vkc
{
    template <typename V>
    class VertexBuffer : public BindableInterface
    {
    public:
        VertexBuffer(VkDeviceSize size);
        VertexBuffer(VkCommandPool cmdPool, V* data, VkDeviceSize size);
        VertexBuffer(const VertexBuffer&) = delete;
        VertexBuffer& operator=(const VertexBuffer&) = delete;

        ~VertexBuffer();

        void Update(VkCommandPool cmdPool, V* data);
        void Bind(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;

    private:
        VkBuffer Buffer;
        VkDeviceMemory Memory;
        VkDeviceSize Size;
    };

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
         * special host visible staging buffer, fill it and then transfer it's contents
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
    template<typename V>
    void VertexBuffer<V>::Bind(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        UNUSED(imageIndex);
        VkBuffer buffers[] = {Buffer};
        VkDeviceSize offsets[] = {0};
        vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
    }
}


#endif //VULKANVERTEXBUFFER_H
