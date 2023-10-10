#ifndef VULKANSHADER_H
#define VULKANSHADER_H

#include "VulkanCore.h"

#include <string>
#include <vector>

namespace vkc
{
    class Shader
    {
    public:
        Shader(ShaderStage type, const std::string& shaderFileName);
        Shader(const Shader&) = delete;
        Shader& operator=(const Shader&) = delete;
        ~Shader();

        VkPipelineShaderStageCreateInfo GetShaderStageCreateInfo() const;

        static VkShaderModule CreateShaderModule(const std::vector<char>& code);

    private:
        ShaderStage Type;
        VkShaderModule Handle;
    };
}

#endif //VULKANSHADER_H
