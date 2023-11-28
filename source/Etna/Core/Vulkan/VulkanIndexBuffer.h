#ifndef VULKANINDEXBUFFER_H
#define VULKANINDEXBUFFER_H

#include "VulkanHeader.h"
#include "VulkanBindableInterface.h"

namespace vkc
{
    class IndexBuffer : public BindableInterface
    {
    public:
        IndexBuffer(VkDeviceSize size);
        IndexBuffer(VkCommandPool cmdPool, uint16_t* data, VkDeviceSize size);
        IndexBuffer(const IndexBuffer&) = delete;
        IndexBuffer& operator=(const IndexBuffer&) = delete;

        ~IndexBuffer() override = default;

        void Update(uint16_t* data);
        void Bind(VkCommandBuffer commandBuffer, uint32_t imageIndex) override;

    private:
        VkBuffer Buffer;
        VkDeviceMemory Memory;
        VkDeviceSize Size;
    };
}

#endif //VULKANINDEXBUFFER_H
