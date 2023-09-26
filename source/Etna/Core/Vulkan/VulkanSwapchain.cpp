#include "VulkanSwapchain.h"

#include "Etna/Core/Utils.h"
#include "VulkanUtils.h"
#include "VulkanContext.h"

#include <limits>

namespace vkc
{
    Swapchain SwapchainBuilder::Build()
    {
        SwapChainSupportDetails swapChainSupport{};
        GetSwapChainSupportDetails(swapChainSupport, Context::GetPhysicalDevice(), Context::GetSurface());

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = ChooseSwapChainExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

        if (swapChainSupport.capabilities.minImageCount > 0 &&
            imageCount < swapChainSupport.capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = Context::GetSurface();
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        auto indices = GetQueueFamilies(Context::GetPhysicalDevice(), Context::GetSurface());
        uint32_t queueFamilyIndices[] = { indices.graphicsFamily.value(),  indices.presentationFamily.value() };
        if (indices.graphicsFamily != indices.presentationFamily)
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }

        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = OldSwapChain;

        Swapchain swapchain;
        if (vkCreateSwapchainKHR(Context::GetDevice(), &createInfo, nullptr, &swapchain.Handle) != VK_SUCCESS)
        {
            Error("Failed to create swap chain.");
        }

        imageCount = 0;
        vkGetSwapchainImagesKHR(Context::GetDevice(), swapchain.Handle, &imageCount, nullptr);

        swapchain.Images.resize(imageCount);
        vkGetSwapchainImagesKHR(Context::GetDevice(), swapchain.Handle, &imageCount, swapchain.Images.data());

        swapchain.ImageFormat = surfaceFormat.format;
        swapchain.Extent = extent;
    }


    VkSurfaceFormatKHR SwapchainBuilder::ChooseSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats)
    {
        for (const auto& format : formats)
        {
            if (format.format == VK_FORMAT_R8G8B8A8_SRGB && format.colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR)
            {
                return format;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR SwapchainBuilder::ChooseSwapChainPresentMode(const std::vector<VkPresentModeKHR>& presentModes)
    {
        for (const auto& presentMode : presentModes)
        {
            if (presentMode == VK_PRESENT_MODE_MAILBOX_KHR)
            {
                return presentMode;
            }
        }

        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D SwapchainBuilder::ChooseSwapChainExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
        {
            return capabilities.currentExtent;
        }
        else
        {
            int width{}, height{};
            glfwGetFramebufferSize(Context::GetWindow(), &width, &height);

            VkExtent2D actualExtent = {
                static_cast<uint32_t>(width),
                static_cast<uint32_t>(height)
            };

            actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

            return actualExtent;
        }

        return VkExtent2D();
    }
}