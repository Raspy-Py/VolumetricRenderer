#ifndef VULKANDEBUGMESSENGER_H
#define VULKANDEBUGMESSENGER_H

#include "VulkanHeader.h"

namespace vkc
{
    class DebugMessenger
    {
    public:
        DebugMessenger() = default;
        ~DebugMessenger();

    public:
        VkDebugUtilsMessengerEXT Handle;
    };

    class DebugMessengerBuilder
    {
    public:
        DebugMessengerBuilder() = default;
        ~DebugMessengerBuilder() = default;

        DebugMessenger Build();
    };
}

#endif //VULKANDEBUGMESSENGER_H
