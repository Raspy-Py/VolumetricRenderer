#ifndef VULKANCORE_H
#define VULKANCORE_H

#include "VulkanHeader.h"

#include <vector>
#include <optional>

namespace vkc
{
    /*
     * Enums
     */
    enum class RenderPassType : int
    {
        Graphic = 0,
        Compute = 1,
        MaxEnum = 0x7FFFFFFF,
    };

    enum class ShaderStage : int
    {
        Vertex = VK_SHADER_STAGE_VERTEX_BIT,
        TesselationControl = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
        TesselationEvaluation = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
        Geometry = VK_SHADER_STAGE_GEOMETRY_BIT,
        Fragment = VK_SHADER_STAGE_FRAGMENT_BIT,
        Compute = VK_SHADER_STAGE_COMPUTE_BIT,
        MaxEnum = VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM
    };

    enum class DescriptorType : int
    {
        Sampler = VK_DESCRIPTOR_TYPE_SAMPLER,
        CombinedImageSampler = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        SampledImage = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,
        StorageImage = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
        UniformTexelBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER,
        StorageTexelBuffer = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER,
        UniformBuffer = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
        StorageBuffer = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
        UniformBufferDynamic = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
        StorageBufferDynamic = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
        InputAttachment = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
        InlineUniformBlock = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK,
        AccelerationStructureKHR = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
        AccelerationStructureNV = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV,
        MutableValue = VK_DESCRIPTOR_TYPE_MUTABLE_VALVE,
        InlineUniformBlockEXT = VK_DESCRIPTOR_TYPE_INLINE_UNIFORM_BLOCK_EXT,
        MaxEnum = VK_DESCRIPTOR_TYPE_MAX_ENUM
    };

    /*
     * Structs
     */

    /// Struct holding indices of the different queue families used by application
    struct QueueFamilyIndices
    {
        bool IsValid()
        {
            return GraphicsFamily.has_value() && PresentationFamily.has_value() && TransferFamily.has_value();
        }

        std::optional<uint32_t> TransferFamily;
        std::optional<uint32_t> GraphicsFamily;
        std::optional<uint32_t> PresentationFamily;
    };

    /// Holds swapchain support details obtained from device properties
    struct SwapChainSupportDetails
    {
        VkSurfaceCapabilitiesKHR Capabilities;
        std::vector<VkSurfaceFormatKHR> Formats;
        std::vector<VkPresentModeKHR> PresentModes;
    };

    struct DescriptorPool
    {
        /// Set free to true to allow vkFreeDescriptorSets(...)
        DescriptorPool(DescriptorType type, uint32_t maxSets, bool free = false);
        ~DescriptorPool();

        DescriptorType Type;
        VkDescriptorPool Handle;
    };

    /*
     * Classes
     */



    /*
     * Interfaces
     */

    class DescriptiveBufferInterface
    {
    public:
        virtual VkDescriptorSetLayout CreateDescriptorSetLayout(uint32_t binding, ShaderStage shaderStage) = 0;
        virtual std::vector<VkDescriptorSet> CreateDescriptorSets(VkDescriptorSetLayout layout, vkc::DescriptorPool pool) = 0;
    };

    /*
     * Utility functions
     */

    /// Fill VkBuffer's memory with data of given size
    void FillBuffer(VkDeviceMemory memory, void* data, VkDeviceSize size);

    /// Copies memory region from VkBuffer src to VkBuffer dest
    void CopyBuffer(VkCommandPool cmdPool, VkBuffer src, VkBuffer dest, VkDeviceSize size);

    /// Returns queue family indices on given GPU
    QueueFamilyIndices GetQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface);

    /// Fills debug messenger creation structure.
    void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    /// Returns image format, extent and present mode, supported by a given GPU
    void GetSwapChainSupportDetails(
        SwapChainSupportDetails &swapChainSupportDetails,
        VkPhysicalDevice        device,
        VkSurfaceKHR            surface);


    template <typename BufferObjectType>
    void UpdateBufferDescriptorSets(
        VkBuffer*           buffers,
        VkDescriptorSet*    sets,
        uint32_t            count = 1);

    void UpdateImageDescriptorSets(
        VkSampler*          samplers,
        VkImageView*        views,
        VkImageLayout*      layouts,
        VkDescriptorSet*    sets,
        uint32_t            count = 1);

    void AllocateDescriptorSets(
        VkDescriptorSetLayout   layout,
        VkDescriptorPool        pool,
        VkDescriptorSet*        sets,
        uint32_t                count = 1);

    VkFormat FindDepthFormat();

    VkFormat FindSupportedFormat(
        const std::vector<VkFormat> &candidates,
        VkImageTiling               tiling,
        VkFormatFeatureFlags        features);

    /*
     * Create functions
     */

    VkCommandPool CreateCommandPool(uint32_t queueFamilyIndex);

    VkCommandBuffer CreateCommandBuffer(VkCommandPool commandPool);

    void CreateCommandBuffers(VkCommandPool commandPool, VkCommandBuffer* buffers, uint32_t count);

    uint32_t ChooseMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void CreateSemaphores(VkSemaphore* semaphores, uint32_t count = 1);

    void CreateFences(VkFence* fences, uint32_t count = 1);

    VkDescriptorSetLayout CreateDescriptorSetLayout(
        uint32_t        binding,
        DescriptorType  type,
        ShaderStage     shaderStage);

    void CreateBuffer(
        VkDeviceSize			size,
        VkBufferUsageFlags		usage,
        VkMemoryPropertyFlags	properties,
        VkBuffer&				buffer,
        VkDeviceMemory&			bufferMemory);

    void CreateFramebuffers(
        VkFramebuffer*  framebuffers,
        VkRenderPass    renderPass, 
        VkExtent2D      extent,
        const VkImageView*    colorAttachments,
        const VkImageView*    depthAttachments = nullptr,
        uint32_t        count = 1);
}

#endif //VULKANCORE_H
