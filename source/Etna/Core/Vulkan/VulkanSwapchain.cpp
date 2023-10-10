#include "VulkanSwapchain.h"

#include "Etna/Core/Utils.h"
#include "VulkanCore.h"

#include "VulkanContext.h"

#include <limits>

namespace vkc
{
    bool Swapchain::AcquireNextImage(VkSemaphore semaphore)
    {
        VkResult result = vkAcquireNextImageKHR(
            Context::GetDevice(), Handle, UINT64_MAX,
            semaphore, VK_NULL_HANDLE, &CurrentImage);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            return true;
        }
        else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            Error("Failed to acquire swapchain image.");
        }

        return false;
    }

    bool Swapchain::PresentImage(VkSemaphore semaphore)
    {
        VkPresentInfoKHR presentInfo{};
        presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
        presentInfo.waitSemaphoreCount = 1;
        presentInfo.pWaitSemaphores = &semaphore;

        presentInfo.swapchainCount = 1;
        presentInfo.pSwapchains = &Handle;
        presentInfo.pImageIndices = &CurrentImage;
        presentInfo.pResults = nullptr;

        VkResult result = vkQueuePresentKHR(Context::GetPresentationQueue(), &presentInfo);

        // Check for swapchain result                                           // TODO: handle swapchain rebuild special case
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR /* || SwapChainRebuild */)
        {
            return true;
        }
        else if (result != VK_SUCCESS)
        {
            Error("Failed to present swapchain image.");
        }

        return false;
    }

    Swapchain SwapchainBuilder::Build()
    {
        // Cleanup old swapchain
        if (OldSwapChain != nullptr)
        {
            auto device = Context::GetDevice();
            vkDeviceWaitIdle(device);

            for (size_t i = 0; i < OldSwapChain->ImageViews.size(); i++)
            {
                vkDestroyImageView(device, OldSwapChain->ImageViews[i], Context::GetAllocator());
            }

            vkDestroySwapchainKHR(device, OldSwapChain->Handle, Context::GetAllocator());
        }

        SwapChainSupportDetails swapChainSupport{};
        GetSwapChainSupportDetails(swapChainSupport, Context::GetPhysicalDevice(), Context::GetSurface());

        VkSurfaceFormatKHR surfaceFormat = ChooseSwapChainSurfaceFormat(swapChainSupport.Formats);
        VkPresentModeKHR presentMode = ChooseSwapChainPresentMode(swapChainSupport.PresentModes);
        VkExtent2D extent = ChooseSwapChainExtent(swapChainSupport.Capabilities);

        uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;

        if (swapChainSupport.Capabilities.minImageCount > 0 &&
            imageCount < swapChainSupport.Capabilities.maxImageCount)
        {
            imageCount = swapChainSupport.Capabilities.maxImageCount;
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
        uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(),  indices.PresentationFamily.value() };
        if (indices.GraphicsFamily != indices.PresentationFamily)
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

        createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        Swapchain swapchain;
        if (vkCreateSwapchainKHR(Context::GetDevice(), &createInfo, Context::GetAllocator(), &swapchain.Handle) != VK_SUCCESS)
        {
            Error("Failed to create swapchain.");
        }

        swapchain.ImageCount = 0;
        swapchain.CurrentImage = 0;
        vkGetSwapchainImagesKHR(Context::GetDevice(), swapchain.Handle, &swapchain.ImageCount, nullptr);

        swapchain.Images.resize(swapchain.ImageCount);
        vkGetSwapchainImagesKHR(Context::GetDevice(), swapchain.Handle, &swapchain.ImageCount, swapchain.Images.data());

        swapchain.ImageFormat = surfaceFormat.format;
        swapchain.Extent = extent;

        CreateImageViews(swapchain);

        // We can afford a full copy, as it only owns the handles of Vulkan objects
        // not the objects themselves
        return swapchain;
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

    void SwapchainBuilder::CreateImageViews(vkc::Swapchain &swapchain)
    {
        swapchain.ImageViews.resize(swapchain.Images.size());

        for (size_t i = 0; i < swapchain.Images.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapchain.Images[i];
            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format = swapchain.ImageFormat;

            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.levelCount = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount = 1;

            if (vkCreateImageView(Context::GetDevice(), &createInfo, nullptr, &swapchain.ImageViews[i]) != VK_SUCCESS)
            {
                Error("Failed to create swapchain image view.");
            }
        }
    }
}