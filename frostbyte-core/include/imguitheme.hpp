#pragma once

#ifndef FROSTBYTE_HEADLESS

#include "imgui.h"

namespace frostbyte {

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

}; // namespace frostbyte

#endif
