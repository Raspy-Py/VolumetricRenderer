#include "VulkanIndexBuffer.h"

#include "VulkanUtils.h"
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

        Update(cmdPool, data);
    }

    void IndexBuffer::Update(VkCommandPool cmdPool, uint16_t *data)
    {
        /*
         * Index buffer uses device local memory, which is not visible to the CPU,
         * so it can't be written from it. To put some data into this buffer, we create
         * special host visible staging buffer, fill it and then transfer it's contains
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
        CopyBuffer(cmdPool, stagingBuffer, Buffer, Size);

        vkDestroyBuffer(Context::GetDevice(), stagingBuffer, Context::GetAllocator());
        vkFreeMemory(Context::GetDevice(), stagingBufferMemory, Context::GetAllocator());
    }
}
