#ifndef VULKANTEXTURE_H
#define VULKANTEXTURE_H

#include "VulkanCore.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <string>

namespace vkc
{
    class Texture
    {
    public:
        explicit Texture(const std::string& imagePath);
        ~Texture();

    private:
        int Width;
        int Height;
        int Channels;

        VkImage Image;
        VkDeviceMemory Memory;
    };
}

#endif //VULKANTEXTURE_H
