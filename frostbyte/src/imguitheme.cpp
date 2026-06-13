#include "imguitheme.hpp"

namespace frostbyte {

ImGuiTheme imgui_theme = ImGuiTheme::ClassicSteam;
ImGuiStyle default_imgui_style;

void changeImGuiTheme(ImGuiTheme theme) {
    imgui_theme = theme;
    ImGuiStyle& style = ImGui::GetStyle();
    style = default_imgui_style;

    switch (theme) {
        case ImGuiTheme::Monochrome: // https://gist.github.com/enemymouse/c8aa24e247a1d7b9fc33d45091cbb8f0
            style.Alpha = 1.0;
            // style.WindowFillAlphaDefault = 0.83;
            // style.ChildWindowRounding = 3;
            style.WindowRounding = 3;
            style.GrabRounding = 1;
            style.GrabMinSize = 20;
            style.FrameRounding = 3;

            style.Colors[ImGuiCol_Text] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.00f, 0.40f, 0.41f, 1.00f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.00f, 1.00f, 1.00f, 0.65f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.44f, 0.80f, 0.80f, 0.18f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.44f, 0.80f, 0.80f, 0.27f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.44f, 0.81f, 0.86f, 0.66f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.14f, 0.18f, 0.21f, 1.00f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.00f, 0.68f, 0.68f, 1.00f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.22f, 0.29f, 0.30f, 0.71f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.44f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
            // style.Colors[ImGuiCol_ComboBg] = ImVec4(0.16f, 0.24f, 0.22f, 0.60f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.00f, 1.00f, 1.00f, 0.68f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.00f, 1.00f, 1.00f, 0.36f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.76f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.00f, 0.65f, 0.65f, 0.46f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.01f, 1.00f, 1.00f, 0.43f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.62f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.00f, 1.00f, 1.00f, 0.33f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.42f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
            // style.Colors[ImGuiCol_Column] = ImVec4(0.00f, 0.50f, 0.50f, 0.33f);
            // style.Colors[ImGuiCol_ColumnHovered] = ImVec4(0.00f, 0.50f, 0.50f, 0.47f);
            // style.Colors[ImGuiCol_ColumnActive] = ImVec4(0.00f, 0.70f, 0.70f, 1.00f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 1.00f, 1.00f, 0.54f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.00f, 1.00f, 1.00f, 0.74f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
            // style.Colors[ImGuiCol_CloseButton] = ImVec4(0.00f, 0.78f, 0.78f, 0.35f);
            // style.Colors[ImGuiCol_CloseButtonHovered] = ImVec4(0.00f, 0.78f, 0.78f, 0.47f);
            // style.Colors[ImGuiCol_CloseButtonActive] = ImVec4(0.00f, 0.78f, 0.78f, 1.00f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.00f, 1.00f, 1.00f, 1.00f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.00f, 1.00f, 1.00f, 0.22f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.00f, 0.13f, 0.13f, 0.90f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.04f, 0.10f, 0.09f, 0.51f);
            break;
        case ImGuiTheme::SynV2: // pasted from Synapse V2 source leak
            style.WindowRounding = 0.0f;
            style.FrameRounding = 0.0f;
            style.GrabRounding = 0.0f;
            style.ScrollbarRounding = 0.0f;

            style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.06f, 0.06f, 0.94f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.21f, 0.22f, 0.54f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.40f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.18f, 0.18f, 0.18f, 0.67f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.29f, 0.29f, 0.29f, 1.00f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.86f, 0.86f, 0.86f, 1.00f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.70f, 0.70f, 0.70f, 0.31f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.70f, 0.70f, 0.70f, 0.80f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.48f, 0.50f, 0.52f, 1.00f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.72f, 0.72f, 0.72f, 0.78f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.91f, 0.91f, 0.91f, 0.25f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.81f, 0.81f, 0.81f, 0.67f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.46f, 0.46f, 0.46f, 0.95f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.73f, 0.60f, 0.15f, 1.00f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.87f, 0.87f, 0.87f, 0.35f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
            style.Colors[ImGuiCol_Tab] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.46f, 0.47f, 0.48f, 1.00f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.44f, 0.44f, 0.44f, 0.40f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.42f, 0.42f, 0.42f, 1.00f);
            break;
        case ImGuiTheme::CatppuccinMocha: { // https://github.com/ocornut/imgui/issues/707#issuecomment-3592676777
            // Catppuccin Mocha Palette
            // --------------------------------------------------------
            const ImVec4 base       = ImVec4(0.117f, 0.117f, 0.172f, 1.0f); // #1e1e2e
            const ImVec4 mantle     = ImVec4(0.109f, 0.109f, 0.156f, 1.0f); // #181825
            const ImVec4 surface0   = ImVec4(0.200f, 0.207f, 0.286f, 1.0f); // #313244
            const ImVec4 surface1   = ImVec4(0.247f, 0.254f, 0.337f, 1.0f); // #3f4056
            const ImVec4 surface2   = ImVec4(0.290f, 0.301f, 0.388f, 1.0f); // #4a4d63
            const ImVec4 overlay0   = ImVec4(0.396f, 0.403f, 0.486f, 1.0f); // #65677c
            const ImVec4 overlay2   = ImVec4(0.576f, 0.584f, 0.654f, 1.0f); // #9399b2
            const ImVec4 text       = ImVec4(0.803f, 0.815f, 0.878f, 1.0f); // #cdd6f4
            const ImVec4 subtext0   = ImVec4(0.639f, 0.658f, 0.764f, 1.0f); // #a3a8c3
            const ImVec4 mauve      = ImVec4(0.796f, 0.698f, 0.972f, 1.0f); // #cba6f7
            const ImVec4 peach      = ImVec4(0.980f, 0.709f, 0.572f, 1.0f); // #fab387
            const ImVec4 yellow     = ImVec4(0.980f, 0.913f, 0.596f, 1.0f); // #f9e2af
            const ImVec4 green      = ImVec4(0.650f, 0.890f, 0.631f, 1.0f); // #a6e3a1
            const ImVec4 teal       = ImVec4(0.580f, 0.886f, 0.819f, 1.0f); // #94e2d5
            const ImVec4 sapphire   = ImVec4(0.458f, 0.784f, 0.878f, 1.0f); // #74c7ec
            const ImVec4 blue       = ImVec4(0.533f, 0.698f, 0.976f, 1.0f); // #89b4fa
            const ImVec4 lavender   = ImVec4(0.709f, 0.764f, 0.980f, 1.0f); // #b4befe

            // Main window and backgrounds
            style.Colors[ImGuiCol_WindowBg]             = base;
            style.Colors[ImGuiCol_ChildBg]              = base;
            style.Colors[ImGuiCol_PopupBg]              = surface0;
            style.Colors[ImGuiCol_Border]               = surface1;
            style.Colors[ImGuiCol_BorderShadow]         = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_FrameBg]              = surface0;
            style.Colors[ImGuiCol_FrameBgHovered]       = surface1;
            style.Colors[ImGuiCol_FrameBgActive]        = surface2;
            style.Colors[ImGuiCol_TitleBg]              = mantle;
            style.Colors[ImGuiCol_TitleBgActive]        = surface0;
            style.Colors[ImGuiCol_TitleBgCollapsed]     = mantle;
            style.Colors[ImGuiCol_MenuBarBg]            = mantle;
            style.Colors[ImGuiCol_ScrollbarBg]          = surface0;
            style.Colors[ImGuiCol_ScrollbarGrab]        = surface2;
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = overlay0;
            style.Colors[ImGuiCol_ScrollbarGrabActive]  = overlay2;
            style.Colors[ImGuiCol_CheckMark]            = green;
            style.Colors[ImGuiCol_SliderGrab]           = sapphire;
            style.Colors[ImGuiCol_SliderGrabActive]     = blue;
            style.Colors[ImGuiCol_Button]               = surface0;
            style.Colors[ImGuiCol_ButtonHovered]        = surface1;
            style.Colors[ImGuiCol_ButtonActive]         = surface2;
            style.Colors[ImGuiCol_Header]               = surface0;
            style.Colors[ImGuiCol_HeaderHovered]        = surface1;
            style.Colors[ImGuiCol_HeaderActive]         = surface2;
            style.Colors[ImGuiCol_Separator]            = surface1;
            style.Colors[ImGuiCol_SeparatorHovered]     = mauve;
            style.Colors[ImGuiCol_SeparatorActive]      = mauve;
            style.Colors[ImGuiCol_ResizeGrip]           = surface2;
            style.Colors[ImGuiCol_ResizeGripHovered]    = mauve;
            style.Colors[ImGuiCol_ResizeGripActive]     = mauve;
            style.Colors[ImGuiCol_Tab]                  = surface0;
            style.Colors[ImGuiCol_TabHovered]           = surface2;
            style.Colors[ImGuiCol_TabActive]            = surface1;
            style.Colors[ImGuiCol_TabUnfocused]         = surface0;
            style.Colors[ImGuiCol_TabUnfocusedActive]   = surface1;
            // style.Colors[ImGuiCol_DockingPreview]       = sapphire;
            // style.Colors[ImGuiCol_DockingEmptyBg]       = base;
            style.Colors[ImGuiCol_PlotLines]            = blue;
            style.Colors[ImGuiCol_PlotLinesHovered]     = peach;
            style.Colors[ImGuiCol_PlotHistogram]        = teal;
            style.Colors[ImGuiCol_PlotHistogramHovered] = green;
            style.Colors[ImGuiCol_TableHeaderBg]        = surface0;
            style.Colors[ImGuiCol_TableBorderStrong]    = surface1;
            style.Colors[ImGuiCol_TableBorderLight]     = surface0;
            style.Colors[ImGuiCol_TableRowBg]           = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_TableRowBgAlt]        = ImVec4(1.0f, 1.0f, 1.0f, 0.06f);
            style.Colors[ImGuiCol_TextSelectedBg]       = surface2;
            style.Colors[ImGuiCol_DragDropTarget]       = yellow;
            style.Colors[ImGuiCol_NavHighlight]         = lavender;
            style.Colors[ImGuiCol_NavWindowingHighlight]= ImVec4(1.0f, 1.0f, 1.0f, 0.7f);
            style.Colors[ImGuiCol_NavWindowingDimBg]    = ImVec4(0.8f, 0.8f, 0.8f, 0.2f);
            style.Colors[ImGuiCol_ModalWindowDimBg]     = ImVec4(0.0f, 0.0f, 0.0f, 0.35f);
            style.Colors[ImGuiCol_Text]                 = text;
            style.Colors[ImGuiCol_TextDisabled]         = subtext0;

            // Rounded corners
            style.WindowRounding    = 6.0f;
            style.ChildRounding     = 6.0f;
            style.FrameRounding     = 4.0f;
            style.PopupRounding     = 4.0f;
            style.ScrollbarRounding = 9.0f;
            style.GrabRounding      = 4.0f;
            style.TabRounding       = 4.0f;

            // Padding and spacing
            style.WindowPadding     = ImVec2(8.0f, 8.0f);
            style.FramePadding      = ImVec2(5.0f, 3.0f);
            style.ItemSpacing       = ImVec2(8.0f, 4.0f);
            style.ItemInnerSpacing  = ImVec2(4.0f, 4.0f);
            style.IndentSpacing     = 21.0f;
            style.ScrollbarSize     = 14.0f;
            style.GrabMinSize       = 10.0f;

            // Borders
            style.WindowBorderSize  = 1.0f;
            style.ChildBorderSize   = 1.0f;
            style.PopupBorderSize   = 1.0f;
            style.FrameBorderSize   = 0.0f;
            style.TabBorderSize     = 0.0f;
            break;
        }
        case ImGuiTheme::Gold: // https://github.com/CookiePLMonster/DXHRDC-GFX
            style.FramePadding = ImVec2(4, 2);
            style.ItemSpacing = ImVec2(10, 2);
            style.IndentSpacing = 12;
            style.ScrollbarSize = 10;

            style.WindowRounding = 4;
            style.FrameRounding = 4;
            style.PopupRounding = 4;
            style.ScrollbarRounding = 6;
            style.GrabRounding = 4;
            style.TabRounding = 4;

            style.Colors[ImGuiCol_Text]                   = ImVec4(0.92f, 0.92f, 0.92f, 1.00f);
            style.Colors[ImGuiCol_TextDisabled]           = ImVec4(0.44f, 0.44f, 0.44f, 1.00f);
            style.Colors[ImGuiCol_WindowBg]               = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
            style.Colors[ImGuiCol_ChildBg]                = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            style.Colors[ImGuiCol_PopupBg]                = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
            style.Colors[ImGuiCol_Border]                 = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
            style.Colors[ImGuiCol_BorderShadow]           = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            style.Colors[ImGuiCol_FrameBg]                = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
            style.Colors[ImGuiCol_FrameBgHovered]         = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
            style.Colors[ImGuiCol_FrameBgActive]          = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
            style.Colors[ImGuiCol_TitleBg]                = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
            style.Colors[ImGuiCol_TitleBgActive]          = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
            style.Colors[ImGuiCol_TitleBgCollapsed]       = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
            style.Colors[ImGuiCol_MenuBarBg]              = ImVec4(0.11f, 0.11f, 0.11f, 1.00f);
            style.Colors[ImGuiCol_ScrollbarBg]            = ImVec4(0.06f, 0.06f, 0.06f, 0.53f);
            style.Colors[ImGuiCol_ScrollbarGrab]          = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered]   = ImVec4(0.47f, 0.47f, 0.47f, 1.00f);
            style.Colors[ImGuiCol_ScrollbarGrabActive]    = ImVec4(0.81f, 0.83f, 0.81f, 1.00f);
            style.Colors[ImGuiCol_CheckMark]              = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
            style.Colors[ImGuiCol_SliderGrab]             = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
            style.Colors[ImGuiCol_SliderGrabActive]       = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
            style.Colors[ImGuiCol_Button]                 = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
            style.Colors[ImGuiCol_ButtonHovered]          = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
            style.Colors[ImGuiCol_ButtonActive]           = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
            style.Colors[ImGuiCol_Header]                 = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
            style.Colors[ImGuiCol_HeaderHovered]          = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
            style.Colors[ImGuiCol_HeaderActive]           = ImVec4(0.93f, 0.65f, 0.14f, 1.00f);
            style.Colors[ImGuiCol_Separator]              = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
            style.Colors[ImGuiCol_SeparatorHovered]       = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
            style.Colors[ImGuiCol_SeparatorActive]        = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
            style.Colors[ImGuiCol_ResizeGrip]             = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
            style.Colors[ImGuiCol_ResizeGripHovered]      = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
            style.Colors[ImGuiCol_ResizeGripActive]       = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
            style.Colors[ImGuiCol_Tab]                    = ImVec4(0.51f, 0.36f, 0.15f, 1.00f);
            style.Colors[ImGuiCol_TabHovered]             = ImVec4(0.91f, 0.64f, 0.13f, 1.00f);
            style.Colors[ImGuiCol_TabActive]              = ImVec4(0.78f, 0.55f, 0.21f, 1.00f);
            style.Colors[ImGuiCol_TabUnfocused]           = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
            style.Colors[ImGuiCol_TabUnfocusedActive]     = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
            style.Colors[ImGuiCol_PlotLines]              = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
            style.Colors[ImGuiCol_PlotLinesHovered]       = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
            style.Colors[ImGuiCol_PlotHistogram]          = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
            style.Colors[ImGuiCol_PlotHistogramHovered]   = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
            style.Colors[ImGuiCol_TextSelectedBg]         = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
            style.Colors[ImGuiCol_DragDropTarget]         = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
            style.Colors[ImGuiCol_NavHighlight]           = ImVec4(0.26f, 0.59f, 0.98f, 1.00f);
            style.Colors[ImGuiCol_NavWindowingHighlight]  = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
            style.Colors[ImGuiCol_NavWindowingDimBg]      = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
            style.Colors[ImGuiCol_ModalWindowDimBg]       = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
            break;
        case ImGuiTheme::SonicRiders: // https://github.com/Sewer56/Riders.Tweakbox/tree/master/Submodules/Sewer56.Imgui
            style.FrameRounding = 4.0f;
            style.WindowRounding = 4.0f;
            style.WindowBorderSize = 0.0f;
            style.PopupBorderSize = 0.0f;
            style.GrabRounding = 4.0f;

            style.Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(0.73f, 0.75f, 0.74f, 1.00f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.09f, 0.94f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(0.08f, 0.08f, 0.08f, 0.94f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.84f, 0.66f, 0.66f, 0.40f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(0.84f, 0.66f, 0.66f, 0.67f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(0.47f, 0.22f, 0.22f, 0.67f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(0.47f, 0.22f, 0.22f, 1.00f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.47f, 0.22f, 0.22f, 0.67f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(0.34f, 0.16f, 0.16f, 1.00f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.02f, 0.02f, 0.02f, 0.53f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.51f, 0.51f, 0.51f, 1.00f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(0.71f, 0.39f, 0.39f, 1.00f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(0.84f, 0.66f, 0.66f, 1.00f);
            style.Colors[ImGuiCol_Button] = ImVec4(0.47f, 0.22f, 0.22f, 0.65f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.71f, 0.39f, 0.39f, 0.65f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.50f);
            style.Colors[ImGuiCol_Header] = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.84f, 0.66f, 0.66f, 0.65f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.84f, 0.66f, 0.66f, 0.00f);
            style.Colors[ImGuiCol_Separator] = ImVec4(0.43f, 0.43f, 0.50f, 0.50f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.84f, 0.66f, 0.66f, 0.66f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.84f, 0.66f, 0.66f, 0.66f);
            style.Colors[ImGuiCol_Tab] = ImVec4(0.71f, 0.39f, 0.39f, 0.54f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(0.84f, 0.66f, 0.66f, 0.66f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(0.84f, 0.66f, 0.66f, 0.66f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(0.07f, 0.10f, 0.15f, 0.97f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.26f, 0.42f, 1.00f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(0.26f, 0.59f, 0.98f, 0.35f);
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
            break;
        case ImGuiTheme::ClassicSteam: // by metasprite
            style.Alpha = 1.0;
            style.DisabledAlpha = 0.6000000238418579;
            style.WindowPadding = ImVec2(8.0, 8.0);
            style.WindowRounding = 0.0;
            style.WindowBorderSize = 1.0;
            style.WindowMinSize = ImVec2(32.0, 32.0);
            style.WindowTitleAlign = ImVec2(0.0, 0.5);
            style.WindowMenuButtonPosition = ImGuiDir_Left;
            style.ChildRounding = 0.0;
            style.ChildBorderSize = 1.0;
            style.PopupRounding = 0.0;
            style.PopupBorderSize = 1.0;
            style.FramePadding = ImVec2(4.0, 3.0);
            style.FrameRounding = 0.0;
            style.FrameBorderSize = 1.0;
            style.ItemSpacing = ImVec2(8.0, 4.0);
            style.ItemInnerSpacing = ImVec2(4.0, 4.0);
            style.CellPadding = ImVec2(4.0, 2.0);
            style.IndentSpacing = 21.0;
            style.ColumnsMinSpacing = 6.0;
            style.ScrollbarSize = 14.0;
            style.ScrollbarRounding = 0.0;
            style.GrabMinSize = 10.0;
            style.GrabRounding = 0.0;
            style.TabRounding = 0.0;
            style.TabBorderSize = 0.0;
            // style.TabMinWidthForCloseButton = 0.0;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5, 0.5);
            style.SelectableTextAlign = ImVec2(0.0, 0.0);

            style.Colors[ImGuiCol_Text] = ImVec4(255.f/255.f, 255.f/255.f, 255.f/255.f, 1.f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(127.f/255.f, 127.f/255.f, 127.f/255.f, 1.f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(73.f/255.f, 86.f/255.f, 66.f/255.f, 1.f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(73.f/255.f, 86.f/255.f, 66.f/255.f, 1.f);
            style.Colors[ImGuiCol_PopupBg] = ImVec4(61.f/255.f, 68.f/255.f, 51.f/255.f, 1.f);
            style.Colors[ImGuiCol_Border] = ImVec4(137.f/255.f, 145.f/255.f, 130.f/255.f, 0.5);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(35.f/255.f, 40.f/255.f, 28.f/255.f, 0.5199999809265137);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(61.f/255.f, 68.f/255.f, 51.f/255.f, 1.f);
            style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(68.f/255.f, 76.f/255.f, 58.f/255.f, 1.f);
            style.Colors[ImGuiCol_FrameBgActive] = ImVec4(76.f/255.f, 86.f/255.f, 66.f/255.f, 1.f);
            style.Colors[ImGuiCol_TitleBg] = ImVec4(61.f/255.f, 68.f/255.f, 51.f/255.f, 1.f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(73.f/255.f, 86.f/255.f, 66.f/255.f, 1.f);
            style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.f, 0.f, 0.f, 0.5099999904632568);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(61.f/255.f, 68.f/255.f, 51.f/255.f, 1.f);
            style.Colors[ImGuiCol_ScrollbarBg] = ImVec4(89.f/255.f, 107.f/255.f, 79.f/255.f, 1.f);
            style.Colors[ImGuiCol_ScrollbarGrab] = ImVec4(71.f/255.f, 81.f/255.f, 61.f/255.f, 1.f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(63.f/255.f, 76.f/255.f, 56.f/255.f, 1.f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(58.f/255.f, 68.f/255.f, 53.f/255.f, 1.f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(150.f/255.f, 137.f/255.f, 45.f/255.f, 1.f);
            style.Colors[ImGuiCol_SliderGrab] = ImVec4(89.f/255.f, 107.f/255.f, 79.f/255.f, 1.f);
            style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(137.f/255.f, 145.f/255.f, 130.f/255.f, 0.5);
            style.Colors[ImGuiCol_Button] = ImVec4(73.f/255.f, 86.f/255.f, 66.f/255.f, 0.4000000059604645);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(89.f/255.f, 107.f/255.f, 79.f/255.f, 1.f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(137.f/255.f, 145.f/255.f, 130.f/255.f, 0.5);
            style.Colors[ImGuiCol_Header] = ImVec4(89.f/255.f, 107.f/255.f, 79.f/255.f, 1.f);
            style.Colors[ImGuiCol_HeaderHovered] = ImVec4(89.f/255.f, 107.f/255.f, 79.f/255.f, 0.6000000238418579);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(137.f/255.f, 145.f/255.f, 130.f/255.f, 0.5);
            style.Colors[ImGuiCol_Separator] = ImVec4(35.f/255.f, 40.f/255.f, 28.f/255.f, 1.f);
            style.Colors[ImGuiCol_SeparatorHovered] = ImVec4(137.f/255.f, 145.f/255.f, 130.f/255.f, 1.f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(150.f/255.f, 137.f/255.f, 45.f/255.f, 1.f);
            style.Colors[ImGuiCol_ResizeGrip] = ImVec4(48.f/255.f, 58.f/255.f, 45.f/255.f, 0.0);
            style.Colors[ImGuiCol_ResizeGripHovered] = ImVec4(137.f/255.f, 145.f/255.f, 130.f/255.f, 1.f);
            style.Colors[ImGuiCol_ResizeGripActive] = ImVec4(150.f/255.f, 137.f/255.f, 45.f/255.f, 1.f);
            style.Colors[ImGuiCol_Tab] = ImVec4(89.f/255.f, 107.f/255.f, 79.f/255.f, 1.f);
            style.Colors[ImGuiCol_TabHovered] = ImVec4(137.f/255.f, 145.f/255.f, 130.f/255.f, 0.7799999713897705);
            style.Colors[ImGuiCol_TabActive] = ImVec4(150.f/255.f, 137.f/255.f, 45.f/255.f, 1.f);
            style.Colors[ImGuiCol_TabUnfocused] = ImVec4(61.f/255.f, 68.f/255.f, 51.f/255.f, 1.f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(89.f/255.f, 107.f/255.f, 79.f/255.f, 1.f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(155.f/255.f, 155.f/255.f, 155.f/255.f, 1.f);
            style.Colors[ImGuiCol_PlotLinesHovered] = ImVec4(150.f/255.f, 137.f/255.f, 45.f/255.f, 1.f);
            style.Colors[ImGuiCol_PlotHistogram] = ImVec4(255.f/255.f, 198.f/255.f, 71.f/255.f, 1.f);
            style.Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(255.f/255.f, 153.f/255.f, 0.f, 1.f);
            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(48.f/255.f, 48.f/255.f, 51.f/255.f, 1.f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(79.f/255.f, 79.f/255.f, 89.f/255.f, 1.f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(58.f/255.f, 58.f/255.f, 63.f/255.f, 1.f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.f, 0.f, 0.f, 0.0);
            style.Colors[ImGuiCol_TableRowBgAlt] = ImVec4(255.f/255.f, 255.f/255.f, 255.f/255.f, 0.05999999865889549);
            style.Colors[ImGuiCol_TextSelectedBg] = ImVec4(150.f/255.f, 137.f/255.f, 45.f/255.f, 1.f);
            style.Colors[ImGuiCol_DragDropTarget] = ImVec4(186.f/255.f, 170.f/255.f, 61.f/255.f, 1.f);
            style.Colors[ImGuiCol_NavHighlight] = ImVec4(150.f/255.f, 137.f/255.f, 45.f/255.f, 1.f);
            style.Colors[ImGuiCol_NavWindowingHighlight] = ImVec4(255.f/255.f, 255.f/255.f, 255.f/255.f, 0.699999988079071);
            style.Colors[ImGuiCol_NavWindowingDimBg] = ImVec4(204.f/255.f, 204.f/255.f, 204.f/255.f, 0.2000000029802322);
            style.Colors[ImGuiCol_ModalWindowDimBg] = ImVec4(204.f/255.f, 204.f/255.f, 204.f/255.f, 0.3499999940395355);
            break;
    }
}

}; // namespace frostbyte
