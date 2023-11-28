#ifndef VULKANBINDABLEINTERFACE_H
#define VULKANBINDABLEINTERFACE_H

#include "VulkanHeader.h"

namespace vkc
{
    class BindableInterface
    {
    public:
        BindableInterface() = default;
        virtual ~BindableInterface() = default;

        virtual void Bind(VkCommandBuffer commandBuffer, uint32_t imageIndex) = 0;
    };
}

#endif //VULKANBINDABLEINTERFACE_H
