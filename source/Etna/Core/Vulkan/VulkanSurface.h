#ifndef VULKANSURFACE_H
#define VULKANSURFACE_H

#include "VulkanHeader.h"

namespace vkc
{
    class Surface
    {
    public:
        Surface() = default;
        ~Surface();

    public:
        VkSurfaceKHR Handle;
    };

    class SurfaceBuilder
    {
    public:
        SurfaceBuilder() = default;
        ~SurfaceBuilder() = default;

        Surface Build(VkInstance instance, GLFWwindow* window);
    };

}

#endif //VULKANSURFACE_H
