#ifndef VULKANDEVICE_H
#define VULKANDEVICE_H

#include "VulkanHeader.h"

#include <vector>

namespace vkc
{
    class Device
    {
    public:
        Device() = default;
        ~Device(){ static_assert("IMPLEMENT!");};
    public:
        /*
         * Broke naming convention here, to avoid duplication of "device".
         * Fields are public 'cause they're used A LOT
         */

        VkDevice            Logical;
        VkPhysicalDevice    Physical;

        VkQueue             TransferQueue;
        VkQueue             GraphicsQueue;
        VkQueue             PresentationQueue;
    };

    /*
     * Well, technically speaking physical device can't be "built"
     * and just being selected out of available GPUs, but a lot
     * of other stuff is created here, so let it be a builder
     */
    class DeviceBuilder
    {
    public:
        DeviceBuilder() = default;
        ~DeviceBuilder() = default;

        Device Build();

    private:
        bool CheckDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface);
        bool CheckDeviceExtensionsSupport(VkPhysicalDevice device);
    };
}

#endif //VULKANDEVICE_H
