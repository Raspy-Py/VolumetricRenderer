#include "Etna/Core/Etna.h"

#include "Etna/Layers/Canvas/Canvas.h"

#include "imgui/imgui.h"
#include "loguru/loguru.hpp"

class TestLayer : public Etna::Layer
{
    class ColorEdit3
    {
    public:
        ColorEdit3(float* color)
            : Color(color)
        {
            PrevColor[0] = Color[0];
            PrevColor[1] = Color[1];
            PrevColor[2] = Color[2];
        }

        bool operator()()
        {
            ImGui::SetNextItemWidth(300);
            ImGui::ColorEdit3("##Color", Color);
            bool changed = false;
            if (Color[0] != PrevColor[0] ||
                Color[1] != PrevColor[1] ||
                Color[2] != PrevColor[2])
            {
                changed = true;
            }

            PrevColor[0] = Color[0];
            PrevColor[1] = Color[1];
            PrevColor[2] = Color[2];
            return changed;
        }

    private:
        float PrevColor[3];
        float* Color;
    };

public:
    void Init() override
    {
        Name = loguru::textprintf("TestLayer%d", Counter).c_str();
        Counter++;
    }

    void Render() override
    {
        static ColorEdit3 colorPicker(Color);

        ImGui::Begin(Name.c_str());

        if (ImGui::Button("Add", ImVec2(70, 20))) { AddFunc(); }
        ImGui::SameLine();
        ShouldClose = ImGui::Button("Close", ImVec2(70, 20));
        ImGui::SeparatorText("Canvas Color");
        if (colorPicker()) { Canvas::SetClearColor(Color[0], Color[1], Color[2]); }

        ImGui::End();
    }

    bool Update(float deltaTime) override
    {
        return ShouldClose;
    }

public:
    static uint32_t Counter;
    static std::function<void()> AddFunc;

private:
    float Color[3];
    std::string Name;
    bool ShouldClose = false;
};
uint32_t TestLayer::Counter = 0;
std::function<void()> TestLayer::AddFunc = []{};

// Inspired by Walnut (C) Studio Cherno
Etna::Application* Etna::CreateApplication()
{
    ApplicationInfo appInfo = {
        .Name = "Etna Application"
    };

    auto App = new Etna::Application(appInfo);

    App->PushLayer<TestLayer>();
    App->PushLayer<Canvas>();

    TestLayer::AddFunc = [App]{
        App->PushLayer<Canvas>();
    };

    return App;
}
