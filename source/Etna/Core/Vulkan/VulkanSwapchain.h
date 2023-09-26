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

    private:
        VkSwapchainKHR Handle;
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

        SwapchainBuilder& SetOldSwapchain(VkSwapchainKHR oldSwapchain);

    private:
        VkSurfaceFormatKHR ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
        VkPresentModeKHR ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR> &presentModes);
        VkExtent2D ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    private:
        VkSwapchainKHR  OldSwapChain = VK_NULL_HANDLE;
    };
}

#endif //VULKANSWAPCHAIN_H
