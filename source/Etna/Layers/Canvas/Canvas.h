#ifndef CANVAS_H
#define CANVAS_H

#include "Etna/Core/Etna.h"

#include "Etna/Core/Vulkan/VulkanHeader.h"

#include "imgui/imgui.h"
#include "loguru/loguru.hpp"

#include <optional>


class Canvas : public Etna::Layer
{
public:
    void Init() override;
    void Shutdown() override;

    void Render() override;
    bool Update(float deltaTime) override;

public:
    static VkViewport GetViewport(uint32_t id = 0);
    static void SetClearColor(float r, float g, float b, uint32_t id = 0);

private:
    bool Open = true;
    uint32_t ID = 0;
    std::string Name;

private:
    static uint32_t Counter;

    static struct
    {
        std::vector<ImColor> Backgrounds;
        std::vector<std::optional<VkViewport>> Viewports;
    }StaticData;
};


#endif //CANVAS_H
