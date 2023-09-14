#ifndef VULKANDEVICE_H
#define VULKANDEVICE_H

#include <vulkan/vulkan.h>

class VulkanDevice
{
public:
    VulkanDevice(VkPhysicalDevice physicalDevice, VkDevice logicalDevice);
    ~VulkanDevice() = default;

    VulkanDevice(const VulkanDevice&) = delete;
    VulkanDevice& operator=(const VulkanDevice&) = delete;

    VulkanDevice(VulkanDevice&& old);
    VulkanDevice& operator=(VulkanDevice&& old)
    {
        if (&old == this)
        {
            return *this;
        }

        PhysicalDevice = old.PhysicalDevice;
        LogicalDevice = old.LogicalDevice;

        old.PhysicalDevice = VK_NULL_HANDLE;
        old.LogicalDevice = VK_NULL_HANDLE;

        return *this;
    }


public:
    VkPhysicalDevice PhysicalDevice;
    VkDevice LogicalDevice;
};


#endif //VULKANDEVICE_H
