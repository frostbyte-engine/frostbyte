#include "ui/fontexplorer.hpp"

#include "fontloader.hpp"
#include "imgui.h"
#include "raylib.h"

namespace frostbyte {

Font* chosen_font = nullptr;
bool render_texture_is_loaded = false;
RenderTexture2D* loaded_render_texture = new RenderTexture2D();

void UI_FontExplorer_render(lua_State *L) {
    ImGui::BeginChild("Font List", ImVec2{ImGui::GetContentRegionAvail().x * 0.35f, ImGui::GetContentRegionAvail().y}, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

    const Font* old_chosen_font = chosen_font;

    for (size_t i = 0; i < FontLoader::font_list.size(); i++) {
        auto& font = FontLoader::font_list[i];
        bool is_selected = chosen_font && font == chosen_font;
        static char label[22];
        snprintf(label, 22, "%zu", i);
        if (ImGui::Selectable(label, is_selected))
            chosen_font = font;
    }

    if (old_chosen_font != chosen_font) {
        UnloadRenderTexture(*loaded_render_texture);
        render_texture_is_loaded = false;
    }

    ImGui::EndChild();

    if (chosen_font) {
        ImGui::SameLine();

        ImGui::BeginChild("Font Display", ImVec2{0, 0}, ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

        if (!render_texture_is_loaded) {
            *loaded_render_texture = LoadRenderTexture(1000, 200);
            render_texture_is_loaded = true;
        }

        Vector2 position{ 0.f, 0.f };

        BeginTextureMode(*loaded_render_texture);
        ClearBackground(BLANK);
        DrawTextEx(*chosen_font, "ABCDEFGHIJKLMNOPQRSTUVWXYZ", position, 24, 1.f, RAYWHITE);
        position.y += 24;
        DrawTextEx(*chosen_font, "abcdefghijklmnopqrstuvwxyz", position, 24, 1.f, RAYWHITE);
        position.y += 24;
        DrawTextEx(*chosen_font, "1234567890", position, 24, 1.f, RAYWHITE);
        position.y += 24;
        DrawTextEx(*chosen_font, "!@#$%%^&*()-=_+", position, 24, 1.f, RAYWHITE);
        EndTextureMode();

        ImGui::Image((ImTextureID)(intptr_t)loaded_render_texture->texture.id, ImVec2(loaded_render_texture->texture.width, loaded_render_texture->texture.height), ImVec2(0, 1), ImVec2(1, 0));

        ImGui::EndChild();
    }
}

void UI_FontExplorer_cleanup() {
    if (render_texture_is_loaded)
        UnloadRenderTexture(*loaded_render_texture);
    delete loaded_render_texture;
}

}; // namespace frostbyte
