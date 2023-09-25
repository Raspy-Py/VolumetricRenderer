#ifndef VULKANINSTANCE_H
#define VULKANINSTANCE_H

#include "VulkanHeader.h"
#include <vector>

namespace vkc
{
    class Instance
    {
    public:
        Instance() = default;
        ~Instance() = default;

    public:
        VkInstance Handle;
    };

    class InstanceBuilder
    {
    public:
        InstanceBuilder() = default;
        ~InstanceBuilder() = default;

        Instance Build();

    private:
        std::vector<const char*> GetRequiredExtensions();
        bool CheckInstanceExtensionsSupport(std::vector<const char*>* requiredExtensions);
        bool CheckValidationLayersSupport();
    };
}

#endif //VULKANINSTANCE_H
