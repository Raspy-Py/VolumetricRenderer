#include "VulkanDebugMessenger.h"

#include "Etna/Core/Utils.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"

namespace vkc
{
    DebugMessenger::~DebugMessenger()
    {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Context::GetInstance(), "vkDestroyDebugUtilsMessengerEXT");

        if (func != nullptr)
            func(Context::GetInstance(), Handle, nullptr);
    }

    DebugMessenger DebugMessengerBuilder::Build()
    {
        auto vkCreateDebugUtilsMessenger = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(Context::GetInstance(), "vkCreateDebugUtilsMessengerEXT");

        if (vkCreateDebugUtilsMessenger == nullptr)
        {
            Error("Debug messenger extension is not present.");
        }

        DebugMessenger debugMessenger;
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        PopulateDebugMessengerCreateInfo(createInfo);
        vkCreateDebugUtilsMessenger(Context::GetInstance(), &createInfo, nullptr, &debugMessenger.Handle);
        return debugMessenger;
    }
}