#include "VulkanCore.h"

#include "Etna/Core/Utils.h"

#include <cstring>
#include <vector>
#include <array>

/*
 * Private
 */
namespace vkc
{
    static constexpr uint32_t DebugMessengerSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                                       #ifdef ENABLE_VERBOSE_LOGGING
                                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT |
                                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                                                       #endif
                                                       VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

    /// Debug callback, passed in instance creation structure.
    static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {
        if (DebugMessengerSeverity & messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
        {
            Error("Debug messenger: %s", pCallbackData->pMessage);
        }else if (DebugMessengerSeverity & messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            Warning("Debug messenger: %s", pCallbackData->pMessage);
        }else if (DebugMessengerSeverity & messageSeverity &
                  (VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT))
        {
            InfoLog("Debug messenger: %s", pCallbackData->pMessage);
        }

        return VK_FALSE;
    }
}

/*
 * Public
 */
namespace vkc
{
    VkDescriptorSetLayout CreateDescriptorSetLayout(const std::vector<VkDescriptorSetLayoutBinding>& bindings)
    {
        VkDescriptorSetLayoutCreateInfo layoutInfo{};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(Context::GetDevice(), &layoutInfo, Context::GetAllocator(), &layout) != VK_SUCCESS)
        {
            Error("Failed to create descriptor set layout.");
        }

        return layout;
    }

    void AllocateDescriptorSets(
        VkDescriptorSetLayout   layout,
        VkDescriptorPool        pool,
        VkDescriptorSet*        sets,
        uint32_t                count)
    {
        std::vector<VkDescriptorSetLayout> layouts(count, layout);
        VkDescriptorSetAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = pool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(count);
        allocInfo.pSetLayouts = layouts.data();

        if (vkAllocateDescriptorSets(Context::GetDevice(), &allocInfo, sets) != VK_SUCCESS)
        {
            Error("Failed to allocate descriptor sets.");
        }
    }

    // Bind descriptor sets to certain images
    void UpdateImageDescriptorSets(VkSampler* samplers, VkImageView* views, VkImageLayout* layouts, VkDescriptorSet* sets, uint32_t count)
    {
        for (size_t i = 0; i < count; i++)
        {
            VkDescriptorImageInfo imageInfo = {
                .sampler = samplers[i],
                .imageView = views[i],
                .imageLayout = layouts[i]
            };

            VkWriteDescriptorSet descriptorWrite{};
            descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite.dstSet = sets[i];
            descriptorWrite.dstBinding = 0;
            descriptorWrite.dstArrayElement = 0;
            descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrite.descriptorCount = 1;
            descriptorWrite.pBufferInfo = nullptr;
            descriptorWrite.pImageInfo = &imageInfo;
            descriptorWrite.pTexelBufferView = nullptr;

            vkUpdateDescriptorSets(Context::GetDevice(), 1, &descriptorWrite, 0, nullptr);
        }
    }

    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                           VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = DebugCallback;
    }

    void GetSwapChainSupportDetails(
        SwapChainSupportDetails &swapChainSupportDetails,
        VkPhysicalDevice        device,
        VkSurfaceKHR            surface)
    {
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &swapChainSupportDetails.Capabilities);

        uint32_t formatsCount = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, nullptr);

        if (formatsCount > 0)
        {
            swapChainSupportDetails.Formats.resize(formatsCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatsCount, swapChainSupportDetails.Formats.data());
        }

        uint32_t presentModesCount = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, nullptr);

        if (presentModesCount > 0)
        {
            swapChainSupportDetails.PresentModes.resize(presentModesCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModesCount, swapChainSupportDetails.PresentModes.data());
        }
    }

    QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilyList(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilyList.data());

        for (uint32_t i = 0; i < queueFamilyCount; i++)
        {
            const auto& queueFamily = queueFamilyList[i];

            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.GraphicsFamily = i;
            }

            if (queueFamily.queueFlags & VK_QUEUE_TRANSFER_BIT)
            {
                indices.TransferFamily = i;
            }

            VkBool32 presentationFamilySupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentationFamilySupport);

            if (presentationFamilySupport)
            {
                indices.PresentationFamily = i;
            }

            if (indices.IsValid())
            {
                break;
            }
        }

        return indices;
    }

    VkDescriptorPool CreateDescriptorPool(VkDescriptorType type, uint32_t count)
    {
        VkDescriptorPoolSize poolSize{};
        poolSize.type = type;
        poolSize.descriptorCount = static_cast<uint32_t>(count);

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;
        poolInfo.maxSets = static_cast<uint32_t>(count);


        VkDescriptorPool pool;
        if (vkCreateDescriptorPool(Context::GetDevice(), &poolInfo, nullptr, &pool) != VK_SUCCESS)
        {
            Error("Failed to create descriptor pool.");
        }

        return pool;
    }

    VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex)
    {
        VkCommandPoolCreateInfo commandPoolInfo = {};
        commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        commandPoolInfo.queueFamilyIndex = queueFamilyIndex;

        VkCommandPool commandPool;
        if (vkCreateCommandPool(Context::GetDevice(), &commandPoolInfo, Context::GetAllocator(), &commandPool) != VK_SUCCESS)
        {
            Error("Failed to create command pool.");
        }

        return commandPool;
    }

    VkCommandBuffer CreateCommandBuffer(VkCommandPool commandPool)
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        if (vkAllocateCommandBuffers(Context::GetDevice(), &allocInfo, &commandBuffer))
        {
            Error("Failed to allocate command buffer.");
        }

        return commandBuffer;
    }

    void CreateCommandBuffers(VkCommandPool commandPool, VkCommandBuffer* buffers, uint32_t count)
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = count;

        VkCommandBuffer commandBuffer;
        if (vkAllocateCommandBuffers(Context::GetDevice(), &allocInfo, buffers))
        {
            Error("Failed to allocate %d command buffers.", count);
        }
    }

    uint32_t ChooseMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(Context::GetPhysicalDevice(), &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
        {
            if (typeFilter & (1u << i) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties )
            {
                return i;
            }
        }

        Error("Failed to find suitable memory type.");
    }

    void CreateBuffer(
        VkDeviceSize			size,
        VkBufferUsageFlags		usage,
        VkMemoryPropertyFlags	properties,
        VkBuffer&				buffer,
        VkDeviceMemory&			bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;

        auto indices = GetQueueFamilies(Context::GetPhysicalDevice(), Context::GetSurface());
        uint32_t queueFamilyIndices[] = { indices.GraphicsFamily.value(),  indices.TransferFamily.value() };
        if (indices.GraphicsFamily != indices.TransferFamily)
        {
            bufferInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
            bufferInfo.queueFamilyIndexCount = 2;
            bufferInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else
        {
            bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
            bufferInfo.queueFamilyIndexCount = 0;
            bufferInfo.pQueueFamilyIndices = nullptr;
        }

        auto device = Context::GetDevice();
        if (vkCreateBuffer(device, &bufferInfo, Context::GetAllocator(), &buffer) != VK_SUCCESS)
        {
            Error("Failed to create buffer.");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device, buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = ChooseMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (vkAllocateMemory(device, &allocInfo, Context::GetAllocator(), &bufferMemory) != VK_SUCCESS)
        {
            Error("Failed to allocate memory for buffer.");
        }

        if (vkBindBufferMemory(device, buffer, bufferMemory, 0) != VK_SUCCESS)
        {
            Error("Failed to bind buffer memory.");
        }
    }

    void CreateSemaphores(VkSemaphore* semaphores, uint32_t count)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            VkSemaphoreCreateInfo semaphoreInfo{};
            semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

            if(vkCreateSemaphore(Context::GetDevice(), &semaphoreInfo, Context::GetAllocator(), &semaphores[i]) != VK_SUCCESS)
            {
                Error("Failed to create semaphore.");
            }
        }
    }

    void CreateFences(VkFence* fences, uint32_t count)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            VkFenceCreateInfo fenceInfo{};
            fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
            fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

            if(vkCreateFence(Context::GetDevice(), &fenceInfo, Context::GetAllocator(), &fences[i]) != VK_SUCCESS)
            {
                Error("Failed to create semaphore.");
            }
        }
    }

    void FillBuffer(VkDeviceMemory memory, void* data, VkDeviceSize size)
    {
        void* mapping;
        vkMapMemory(Context::GetDevice(), memory, 0, size, 0, &mapping);
            memcpy(mapping, data, size);
        vkUnmapMemory(Context::GetDevice(), memory);
    }

    void CopyBuffer(VkBuffer src, VkBuffer dest, VkDeviceSize size)
    {
        auto commandBuffer = BeginSingleTimeCommands();
            VkBufferCopy copyRegion{};
            copyRegion.srcOffset = 0;
            copyRegion.dstOffset = 0;
            copyRegion.size = size;
            vkCmdCopyBuffer(commandBuffer, src, dest, 1, &copyRegion);
        EndSingleTimeCommands(commandBuffer);
    }

    VkFormat FindDepthFormat()
    {
        return FindSupportedFormat(
            {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
    }

    bool HasStencilComponent(VkFormat format)
    {
        return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
    }

    VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(Context::GetPhysicalDevice(), format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features)
            {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features)
            {
                return format;
            }
        }

        Error("Failed to find supported format.");
    }

    void CreateFramebuffers(
        VkFramebuffer*  framebuffers,
        VkRenderPass    renderPass,
        VkExtent2D      extent, 
        const VkImageView*    colorAttachments,
        const VkImageView*    depthAttachment,
        uint32_t        count)
    {
        std::vector<VkImageView> attachments(1);
        if (depthAttachment)
        {
            attachments.resize(2);
        }

        for (uint32_t i = 0; i < count; i++)
        {
            attachments[0] = colorAttachments[i];
            if (depthAttachment)
            {
                attachments[1] = *depthAttachment;
            }

            VkFramebufferCreateInfo framebufferInfo{};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = renderPass;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = extent.width;
			framebufferInfo.height = extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(Context::GetDevice(), &framebufferInfo, Context::GetAllocator(), &framebuffers[i]) != VK_SUCCESS)
			{
				Error("Failed to create framebuffer.");
			}
        }
    }

    void CreateImage(
        uint32_t width,
        uint32_t height,
        uint32_t depth,
        VkImageType type,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage &image,
        VkDeviceMemory &imageMemory)
    {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = type;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = depth;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        auto device = Context::GetDevice();

        if (vkCreateImage(device, &imageInfo, nullptr, &image) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = ChooseMemoryType(memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(device, image, imageMemory, 0);
    }

    void CreateImage2D(
        uint32_t width,
        uint32_t height,
        VkFormat format,
        VkImageTiling tiling,
        VkImageUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory)
    {
        CreateImage(width, height, 1, VK_IMAGE_TYPE_2D, format, tiling, usage, properties, image, imageMemory);
    }

    VkCommandBuffer BeginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = Context::GetTransferCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer{};
        vkAllocateCommandBuffers(Context::GetDevice(), &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);

        return commandBuffer;
    }

    void EndSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(Context::GetTransferQueue(), 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(Context::GetTransferQueue());
        vkFreeCommandBuffers(Context::GetDevice(), Context::GetTransferCommandPool(), 1, &commandBuffer);
    }

    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        auto commandBuffer = BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        if (newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
            if (HasStencilComponent(format))
            {
                barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
            }
        }else
        {
            barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
                  newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }else if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
                  newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }else
        {
            Error("Unsupported layout transition: %i->%i.", (int) oldLayout, (int) newLayout);
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage,
            destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        EndSingleTimeCommands(commandBuffer);
    }

    void CopyBufferImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
    {
        auto commandBuffer = BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, 1};

        vkCmdCopyBufferToImage(
            commandBuffer,
            buffer,
            image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );

        EndSingleTimeCommands(commandBuffer);
    }

    VkImageView CreateImageView(
        VkImage image,
        VkFormat format,
        VkImageViewType type,
        VkImageAspectFlags aspectFlags)
    {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(Context::GetDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
        {
            Error("failed to create texture image view.");
        }

        return imageView;
    }

    VkSampler CreateSampler()
    {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;

        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(Context::GetPhysicalDevice(), &properties);

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;

        VkSampler sampler;
        if (vkCreateSampler(Context::GetDevice(), &samplerInfo, Context::GetAllocator(), &sampler) != VK_SUCCESS)
        {
            Error("Failed to create texture sampler.");
        }

        return sampler;
    }

    VkDescriptorSetLayoutBinding CreateDescriptorSetLayoutBinding(
        uint32_t binding,
        uint32_t count,
        ShaderStage shaderStage,
        DescriptorType type)
    {
        return {
            .binding = binding,
            .descriptorType = static_cast<VkDescriptorType>(type),
            .descriptorCount = count,
            .stageFlags = static_cast<VkShaderStageFlags>(shaderStage),
            .pImmutableSamplers = nullptr
        };
    }
}
