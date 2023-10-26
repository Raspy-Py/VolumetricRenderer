#include "VulkanSurface.h"

#include "Etna/Core/Utils.h"
#include "VulkanContext.h"

namespace vkc
{
    Surface::~Surface()
    {
        vkDestroySurfaceKHR(Context::GetInstance(), Handle, nullptr);
    }

    Surface SurfaceBuilder::Build()
    {
        Surface surface{};
        if (glfwCreateWindowSurface(Context::GetInstance(), Window, Context::GetAllocator(), &surface.Handle) != VK_SUCCESS)
        {
            Error("Failed to create window surface.");
        }

        return surface;
    }

    SurfaceBuilder& SurfaceBuilder::SetWindow(GLFWwindow* window)
    {
        Window = window;
        return *this;
    }
}