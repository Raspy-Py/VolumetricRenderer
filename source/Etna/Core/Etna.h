#ifndef ETNA_H
#define ETNA_H

#include "Vulkan/VulkanHeader.h"

#include <memory>

// Singleton without Get() method.
class Etna
{
private:
    Etna() = default;
    ~Etna() = default;

public:
private:
    static Etna* StaticInstance;

    // Using vanilla Vulkan objects for now
    VkDescriptorPool ImGuiDescriptorPool;


};


#endif //ETNA_H
