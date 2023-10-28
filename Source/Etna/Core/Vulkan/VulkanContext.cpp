#include "VulkanContext.h"

#include "Etna/Core/Utils.h"
#include "VulkanCore.h"
#include <imgui.h>

#include <set>

namespace vkc
{
    Context* Context::Singleton = nullptr;

    void Context::Create()
    {
        if (Singleton != nullptr)
        {
            return;
        }

        Singleton = new Context();

        // Create window with Vulkan context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        Singleton->GWindow = glfwCreateWindow(1280, 720, "Etna App", nullptr, nullptr);

        // Don't change initialization order!
        Singleton->GInstance = InstanceBuilder{}.Build();
        Singleton->GSurface = CreateSurface(Context::GetWindow());
        Singleton->GDevice = DeviceBuilder{}.Build();

        auto indices = GetQueueFamilies(Context::GetPhysicalDevice(), Context::GetSurface());
        Singleton->GTransferCommandPool = CreateCommandPool(indices.TransferFamily.value());
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

    void Context::StartFrame()
    {}

    void Context::EndFrame()
    {}

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
        return Get().GSurface;
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

    VkQueue Context::GetTransferQueue()
    {
        return Get().GDevice.TransferQueue;
    }

    VkQueue Context::GetGraphicsQueue()
    {
        return Get().GDevice.GraphicsQueue;
    }

    VkQueue Context::GetPresentationQueue()
    {
        return Get().GDevice.PresentationQueue;
    }

    VkCommandPool Context::GetTransferCommandPool()
    {
        return Get().GTransferCommandPool;
    }
}
