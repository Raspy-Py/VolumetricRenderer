#include "VulkanDevice.h"

#include "VulkanUtils.h"
#include "VulkanContext.h"
#include "Etna/Core/Utils.h"

#include <set>

namespace vkc
{
    static const std::vector<const char*> DeviceTypes = {
        "Other",
        "Integrated GPU",
        "Discrete GPU",
        "Virtual GPU",
        "CPU"
    };

    static const std::vector<const char*> DeviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    Device DeviceBuilder::Build()
    {
        Device device{};
        auto surface = Context::GetSurface();
        auto instance = Context::GetInstance();

        // Selecting physical device
        {
            uint32_t GPUsCount = 0;
            vkEnumeratePhysicalDevices(instance, &GPUsCount, nullptr);

            if (GPUsCount == 0)
            {
                Error("Failed to find device with Vulkan support.");
            }

            std::vector<VkPhysicalDevice> GPUs(GPUsCount);
            vkEnumeratePhysicalDevices(instance, &GPUsCount, GPUs.data());

            for (const auto &GPU: GPUs)
            {
                if (CheckDeviceSuitable(GPU, surface))
                {
                    device.Physical = GPU;
                    break;
                }
            }

            if (device.Physical == VK_NULL_HANDLE)
            {
                Error("Failed to find suitable device.");
            }else
            {
                VkPhysicalDeviceProperties deviceProperties = {};
                vkGetPhysicalDeviceProperties(device.Physical, &deviceProperties);

                InfoLog("Found suitable device:");
                InfoLog("%4s: %s", "Name", deviceProperties.deviceName);
                InfoLog("%4s: %s", "Type", DeviceTypes[deviceProperties.deviceType]);
                InfoLog("%4s: %d", "ID", deviceProperties.deviceID);
            }
        }

        // Creating logical device and queues
        {
            QueueFamilyIndices indices = GetQueueFamilies(device.Physical, surface);
            std::set<uint32_t> queueFamilyIndices = {
                indices.graphicsFamily.value(),
                indices.presentationFamily.value(),
                indices.transferFamily.value()
            };
            std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

            float priority = 1.0f;
            for (uint32_t queueFamilyIndex : queueFamilyIndices)
            {
                VkDeviceQueueCreateInfo queueCreateInfo{};
                queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
                queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
                queueCreateInfo.queueCount = 1;
                queueCreateInfo.pQueuePriorities = &priority;

                queueCreateInfos.push_back(queueCreateInfo);
            }

            VkDeviceCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
            createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
            createInfo.pQueueCreateInfos = queueCreateInfos.data();
            createInfo.enabledExtensionCount = static_cast<uint32_t>(DeviceExtensions.size());
            createInfo.ppEnabledExtensionNames = DeviceExtensions.data();
            createInfo.enabledLayerCount = 0;

            VkPhysicalDeviceFeatures deviceFeatures{};

            createInfo.pEnabledFeatures = &deviceFeatures;

            if (vkCreateDevice(device.Physical, &createInfo, nullptr, &device.Logical) != VK_SUCCESS)
            {
                Error("Failed to create a logical device.");
            }

            vkGetDeviceQueue(device.Logical, indices.transferFamily.value(), 0, &device.TransferQueue);
            vkGetDeviceQueue(device.Logical, indices.graphicsFamily.value(), 0, &device.GraphicsQueue);
            vkGetDeviceQueue(device.Logical, indices.presentationFamily.value(), 0, &device.PresentationQueue);
        }

        return device;
    }

    bool DeviceBuilder::CheckDeviceExtensionsSupport(VkPhysicalDevice device)
    {
        uint32_t extensionsCount = 0;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(DeviceExtensions.begin(), DeviceExtensions.end());

        for (const auto& extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    bool DeviceBuilder::CheckDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        // Check whether needed queues are supported
        auto indices = GetQueueFamilies(device, surface);

        bool extensionsSupported = CheckDeviceExtensionsSupport(device);
        bool swapChainAdequate = false;
        if (extensionsSupported)
        {
            SwapChainSupportDetails swapChainDetails{};
            GetSwapChainSupportDetails(swapChainDetails, device, surface);
            swapChainAdequate = !swapChainDetails.formats.empty() && !swapChainDetails.presentModes.empty();
        }

        return extensionsSupported && swapChainAdequate && indices.IsValid();
    }
}