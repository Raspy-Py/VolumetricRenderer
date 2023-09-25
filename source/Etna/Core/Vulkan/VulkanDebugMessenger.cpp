#include "VulkanDebugMessenger.h"

#include "Etna/Core/Utils.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace vkc
{
    DebugMessenger::~DebugMessenger()
    {
        auto instance = Context::Get().GInstance.Handle;
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr)
            func(instance, Handle, nullptr);
    }

    DebugMessenger DebugMessengerBuilder::Build(VkInstance instance)
    {
        auto vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");

        if (vkCreateDebugUtilsMessenger == nullptr)
        {
            Error("Debug messenger extension is not present.");
        }

        DebugMessenger debugMessenger;
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        PopulateDebugMessengerCreateInfo(createInfo);
        vkCreateDebugUtilsMessenger(instance, &createInfo, nullptr, &debugMessenger.Handle);
        return debugMessenger;
    }
}