#include "VulkanSurface.h"

#include "Etna/Core/Utils.h"
#include "VulkanContext.h"

namespace vkc
{
    Surface::~Surface()
    {
        vkDestroySurfaceKHR(Context::Get().GInstance.Handle, Handle, nullptr);
    }

    Surface SurfaceBuilder::Build(VkInstance instance, GLFWwindow *window)
    {
        Surface surface;
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface.Handle) != VK_SUCCESS)
        {
            Error("Failed to create window surface.");
        }

        return surface;
    }
}