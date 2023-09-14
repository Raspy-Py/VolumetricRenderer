#include "VulkanDevice.h"

VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice, VkDevice logicalDevice)
    : PhysicalDevice(physicalDevice), LogicalDevice(logicalDevice)
{}

VulkanDevice::VulkanDevice(VulkanDevice &&old)
{
    PhysicalDevice = old.PhysicalDevice;
    LogicalDevice = old.LogicalDevice;

    old.PhysicalDevice = VK_NULL_HANDLE;
    old.LogicalDevice = VK_NULL_HANDLE;
}
