/*
 * Swapchain used to present images on the screen
 * and handling multiple frames on flight.
 * In my implementation there is only one
 * swapchain per Context, in other words -
 * only one swapchain instance is allowed.
 */

#ifndef VULKANSWAPCHAIN_H
#define VULKANSWAPCHAIN_H

#include "VulkanHeader.h"

#include <vector>

namespace vkc
{
    class Swapchain
    {
        friend class SwapchainBuilder;

    public:
        Swapchain() = default;
        ~Swapchain() = default;

        // Both methods return true, if swapchain needs to be recreated
        // otherwise return false
        bool AcquireNextImage(VkSemaphore semaphore);
        bool PresentImage(VkSemaphore semaphore);

        [[nodiscard]] VkFormat GetFormat() const        { return ImageFormat; }
        [[nodiscard]] uint32_t GetImageCount() const    { return ImageCount; }
        [[nodiscard]] uint32_t GetCurrentImage() const  { return CurrentImage; }
        [[nodiscard]] VkExtent2D GetExtent() const      { return Extent; }
        [[nodiscard]] const std::vector<VkImageView>& GetImageViews() const { return ImageViews; }

    private:
        VkSwapchainKHR Handle;

        uint32_t ImageCount;
        uint32_t CurrentImage;
        VkFormat ImageFormat;
        VkExtent2D Extent;
        std::vector<VkImage> Images;
        std::vector<VkImageView> ImageViews;
    };

    class SwapchainBuilder
    {
    public:
        SwapchainBuilder() = default;
        ~SwapchainBuilder() = default;

        Swapchain Build();

        SwapchainBuilder& SetOldSwapchain(Swapchain* oldSwapchain);

    private:
        VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
        VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> &presentModes);
        VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities);

        void CreateImageViews(Swapchain& swapchain);

    private:
        Swapchain* OldSwapChain = nullptr;
    };
}

#endif //VULKANSWAPCHAIN_H
