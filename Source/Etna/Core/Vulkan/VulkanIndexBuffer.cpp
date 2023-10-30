#include "VulkanIndexBuffer.h"

#include "VulkanCore.h"
#include "VulkanContext.h"

namespace vkc
{
    IndexBuffer::IndexBuffer(VkDeviceSize size)
        : Size(size * sizeof(uint16_t))
    {
        CreateBuffer(
            Size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            Buffer,
            Memory
        );
    }

    IndexBuffer::IndexBuffer(VkCommandPool cmdPool, uint16_t *data, VkDeviceSize size)
        : Size(size * sizeof(uint16_t))
    {
        CreateBuffer(
            Size,
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
            VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            Buffer,
            Memory
        );

        Update(data);
    }

    void IndexBuffer::Update(uint16_t *data)
    {
        /*
         * Index buffer uses device local memory, which is not visible to the CPU,
         * so it can't be written from it. To put some data into this buffer, we create
         * special host visible staging buffer, fill it and then transfer it's contents
         * into the index buffer.
         */

        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        CreateBuffer(
            Size,
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer,
            stagingBufferMemory
        );

        FillBuffer(stagingBufferMemory, data, Size);
        CopyBuffer(stagingBuffer, Buffer, Size);

        vkDestroyBuffer(Context::GetDevice(), stagingBuffer, Context::GetAllocator());
        vkFreeMemory(Context::GetDevice(), stagingBufferMemory, Context::GetAllocator());
    }

    void IndexBuffer::Bind(VkCommandBuffer commandBuffer, uint32_t imageIndex)
    {
        vkCmdBindIndexBuffer(commandBuffer, Buffer, 0, VK_INDEX_TYPE_UINT16);
    }
}
