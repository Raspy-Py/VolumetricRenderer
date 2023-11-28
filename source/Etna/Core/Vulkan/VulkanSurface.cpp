#include "VulkanSurface.h"

#include "Etna/Core/Utils.h"
#include "VulkanContext.h"

namespace vkc
{
    VkSurfaceKHR CreateSurface(GLFWwindow* window)
    {
        VkSurfaceKHR surface;
        if (glfwCreateWindowSurface(Context::GetInstance(), window, Context::GetAllocator(), &surface) != VK_SUCCESS)
        {
            Error("Failed to create window surface.");
        }

        return surface;
    }
}