#include "Etna/Core/Etna.h"

#include "Etna/Layers/Canvas/Canvas.h"

#include "imgui/imgui.h"
#include "loguru/loguru.hpp"

// Inspired by Walnut (C) Studio Cherno
Etna::Application* Etna::CreateApplication()
{
    ApplicationInfo appInfo = {
        .Name = "Etna Application"
    };

    auto App = new Etna::Application(appInfo);

    App->PushLayer<Canvas>();

    return App;
}
