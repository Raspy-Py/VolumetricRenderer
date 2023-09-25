#ifndef VULKANCONTEXT_H
#define VULKANCONTEXT_H

#include "VulkanHeader.h"
#include "VulkanInstance.h"
#include "VulkanSurface.h"
#include "VulkanDevice.h"
#include "VulkanDebugMessenger.h"

namespace vkc
{
    /*
     * Some data like VkInstance, VkPhysicalDevice, VkDevice etc.
     * just have to be globally accessible. There is no work around.
     * Long live the Global Vulkan Context!
     */

    class Context
    {
    public:
        static void Create(GLFWwindow* window);
        static void Destroy();
        static Context& Get();

    public:
        Instance    GInstance;
        Surface     GSurface;
        Device      GDevice;

        DebugMessenger  GDebugMessenger;

    private:
        Context();

    private:
        static Context* Singleton;
    };
}

#endif //VULKANCONTEXT_H
