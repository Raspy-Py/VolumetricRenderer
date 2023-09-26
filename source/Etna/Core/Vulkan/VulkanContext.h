/*
 * Some data like VkInstance, VkPhysicalDevice, VkDevice etc.
 * just have to be globally accessible. There is no work around.
 * Long live the Global Vulkan Context!
 */

#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H

#include "VulkanHeader.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanDevice.h"
#include "VulkanDebugMessenger.h"

namespace vkc
{
    class Context
    {
    public:
        static void Create(GLFWwindow *window);
        static void Destroy();
        static Context &Get();

    public:
        static VkInstance GetInstance();
        static VkDevice GetDevice();
        static VkPhysicalDevice GetPhysicalDevice();
        static VkSurfaceKHR GetSurface();
        static VkAllocationCallbacks* GetAllocator();
        static GLFWwindow* GetWindow();

    private:
        Instance GInstance;
        Surface GSurface;
        Device GDevice;

        DebugMessenger GDebugMessenger;
        GLFWwindow* GWindow;

    private:
        Context();

    private:
        static Context *Singleton;
    };
}

#endif //VULKANCONTEXT_H
