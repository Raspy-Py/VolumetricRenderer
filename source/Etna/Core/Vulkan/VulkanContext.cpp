#include "VulkanContext.h"

#include "Etna/Core/Utils.h"
#include "imgui/imgui.h"

#include <set>
#include <limits>

namespace vkc
{
    Context* Context::Singleton = nullptr;

    void Context::Create(GLFWwindow *window)
    {
        if (Singleton != nullptr)
        {
            return;
        }

        Singleton = new Context();

        Singleton->GWindow = window;
        // Don't change initialization order!
        Singleton->GInstance = InstanceBuilder{}.Build();
        Singleton->GDebugMessenger = DebugMessengerBuilder{}.Build();
        Singleton->GSurface = SurfaceBuilder{}
            .SetWindow(window)
            .Build();
        Singleton->GDevice = DeviceBuilder{}.Build();
    }

    void Context::Destroy()
    {
        delete Singleton;
    }

    Context& Context::Get()
    {
        assert(Singleton != nullptr && "Context being reached before creation.");

        return *Singleton;
    }

    VkInstance Context::GetInstance()
    {
        return Get().GInstance.Handle;
    }

    VkDevice Context::GetDevice()
    {
        return Get().GDevice.Logical;
    }

    VkPhysicalDevice Context::GetPhysicalDevice()
    {
        return Get().GDevice.Physical;
    }

    VkSurfaceKHR Context::GetSurface()
    {
        return Get().GSurface.Handle;
    }

    VkAllocationCallbacks* Context::GetAllocator()
    {
        /*
         * TODO: explore memory management
         */

        return nullptr;
    }

    GLFWwindow* Context::GetWindow()
    {
        return Get().GWindow;
    }

}
