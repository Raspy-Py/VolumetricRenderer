#include "Canvas.h"

static const ImColor DefaultBackground = ImColor(0.49f, 0.76f, 1.0f, 1.0f);

uint32_t Canvas::Counter = 0 ;
decltype(Canvas::StaticData) Canvas::StaticData = {};

void Canvas::Init()
{
    ID = Counter++;
    Name = loguru::textprintf("Canvas_%d", ID).c_str();

    StaticData.Viewports.emplace_back(std::nullopt);
    StaticData.Backgrounds.emplace_back(DefaultBackground);
}

void Canvas::Shutdown()
{
    // We don't want anything to be drawn to a closed Canvas
    StaticData.Viewports[ID] = std::nullopt;
}

void Canvas::Render()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_WindowBg, StaticData.Backgrounds[ID].Value);
    ImGui::Begin(Name.c_str(), &Open, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse);

    // Updating viewport data
    {
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();

        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        vMax.x += ImGui::GetWindowPos().x;
        vMax.y += ImGui::GetWindowPos().y;

        StaticData.Viewports[ID] = {
            .x = vMin.x,
            .y = vMin.y,
            .width = vMax.x - vMin.x,
            .height = vMax.y - vMin.y,
            .minDepth = 0,
            .maxDepth = 1
        };

        // Viewport preview
        //ImGui::GetForegroundDrawList()->AddRect( vMin, vMax, IM_COL32( 255, 255, 0, 255 ) );
    }

    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
}

bool Canvas::Update(float deltaTime)
{
    return !Open;
}

VkViewport Canvas::GetViewport(uint32_t id)
{
    if (id < Counter && StaticData.Viewports[id].has_value()) { return StaticData.Viewports[id].value(); }
    Error("Canvas_%d is unavailable.", id);
}

void Canvas::SetClearColor(float r, float g, float b, uint32_t id)
{
    if (id < Counter) { StaticData.Backgrounds[id] = {r, g, b, 1.0f}; }
    else { Error("Canvas_%d is unavailable.", id); }
}
