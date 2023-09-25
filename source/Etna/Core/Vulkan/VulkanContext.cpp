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

        Singleton->GInstance = InstanceBuilder{}.Build();
        Singleton->GDebugMessenger = DebugMessengerBuilder{}.Build(Singleton->GInstance.Handle);
        Singleton->GSurface = SurfaceBuilder{}.Build(Singleton->GInstance.Handle, window);
        Singleton->GDevice = DeviceBuilder{}.Build(Singleton->GInstance.Handle, Singleton->GSurface.Handle);
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
}
