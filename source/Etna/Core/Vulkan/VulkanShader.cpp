#include "VulkanShader.h"

#include "VulkanContext.h"
#include "Etna/Core/Utils.h"

namespace vkc
{
    Shader::Shader(vkc::ShaderStage type, const std::string &shaderFileName)
        : Type(type)
    {
        std::vector<char> shaderCode;

        ReadFile(shaderFileName, shaderCode);

        Handle = CreateShaderModule(shaderCode);
    }

    Shader::~Shader()
    {
        vkDestroyShaderModule(Context::GetDevice(), Handle, Context::GetAllocator());
    }

    VkPipelineShaderStageCreateInfo Shader::GetShaderStageCreateInfo() const
    {
        VkPipelineShaderStageCreateInfo shaderStageInfo = {};
        shaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfo.stage = static_cast<VkShaderStageFlagBits>(Type);
        shaderStageInfo.module = Handle;
        shaderStageInfo.pName = "main";

        return shaderStageInfo;
    }

    VkShaderModule Shader::CreateShaderModule(const std::vector<char> &code)
    {
        VkShaderModuleCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        VkShaderModule shaderModule{};
        if (vkCreateShaderModule(Context::GetDevice(), &createInfo, Context::GetAllocator(), &shaderModule) != VK_SUCCESS)
        {
            Error("Failed to create shader module.");
        }

        return shaderModule;
    }
}