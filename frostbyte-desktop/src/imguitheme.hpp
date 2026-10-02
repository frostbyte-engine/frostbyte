#pragma once

#include "imgui.h"

enum class ImGuiTheme {
    Monochrome,
    SynV2,
    CatppuccinMocha,
    Gold,
    SonicRiders,
    ClassicSteam
};

extern ImGuiTheme imgui_theme;
extern ImGuiStyle default_imgui_style;
void changeImGuiTheme(ImGuiTheme theme);
