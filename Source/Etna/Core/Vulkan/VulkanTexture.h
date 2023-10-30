#ifndef VULKANTEXTURE_H
#define VULKANTEXTURE_H

#include "VulkanCore.h"

#include <stb_image.h>
#include <string>

namespace vkc
{
    class Texture
    {
    public:
        explicit Texture(const std::string& imagePath);
        ~Texture();

        [[nodiscard]] VkImageView GetView() const { return ImageView; }
        [[nodiscard]] VkSampler GetSampler() const { return Sampler; }

    private:
        int Width;
        int Height;
        int Channels;

        VkImage Image;
        VkSampler Sampler;
        VkImageView ImageView;
        VkDeviceMemory Memory;
    };
}

#endif //VULKANTEXTURE_H
