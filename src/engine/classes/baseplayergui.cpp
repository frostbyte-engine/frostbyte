#include "engine/classes/baseplayergui.hpp"
#include "engine/classes/camera.hpp"
#include "engine/classes/guibutton.hpp"
#include "engine/classes/instance.hpp"
#include "engine/classes/userinputservice.hpp"
#include "engine/datatypes/font.hpp"
#include "engine/datatypes/rbxscriptsignal.hpp"
#include "engine/datatypes/udim2.hpp"

#include "common.hpp"
#include "ui/ui.hpp"

#include "lua.h"
#include "lualib.h"
#include "raylib.h"
#include "rlgl.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <mutex>

namespace frostbyte {

std::vector<std::shared_ptr<rbxInstance>> gui_storage_list;

// modification of DrawRectanglePro that doesn't fill and allows thickness
std::array<Vector2, 4> getRectangleLinesPro(Rectangle rec, Vector2 origin, float rotation) {
    // FIXME: this leaves gaps in the corners

    Vector2 topLeft = { 0 };
    Vector2 topRight = { 0 };
    Vector2 bottomLeft = { 0 };
    Vector2 bottomRight = { 0 };

    // Only calculate rotation if needed
    if (rotation == 0.0f) {
        float x = rec.x - origin.x;
        float y = rec.y - origin.y;
        topLeft = (Vector2){ x, y };
        topRight = (Vector2){ x + rec.width, y };
        bottomLeft = (Vector2){ x, y + rec.height };
        bottomRight = (Vector2){ x + rec.width, y + rec.height };
    } else {
        float sinRotation = sinf(rotation*DEG2RAD);
        float cosRotation = cosf(rotation*DEG2RAD);
        float x = rec.x;
        float y = rec.y;
        float dx = -origin.x;
        float dy = -origin.y;

        topLeft.x = x + dx*cosRotation - dy*sinRotation;
        topLeft.y = y + dx*sinRotation + dy*cosRotation;

        topRight.x = x + (dx + rec.width)*cosRotation - dy*sinRotation;
        topRight.y = y + (dx + rec.width)*sinRotation + dy*cosRotation;

        bottomLeft.x = x + dx*cosRotation - (dy + rec.height)*sinRotation;
        bottomLeft.y = y + dx*sinRotation + (dy + rec.height)*cosRotation;

        bottomRight.x = x + (dx + rec.width)*cosRotation - (dy + rec.height)*sinRotation;
        bottomRight.y = y + (dx + rec.width)*sinRotation + (dy + rec.height)*cosRotation;
    }


    return {topLeft, topRight, bottomRight, bottomLeft};
}
void DrawRotatedRectangleLines(Vector2 position, Vector2 size, float rotation, float thickness, Color color) {
    Vector2 center = { position.x + size.x / 2.0f, position.y + size.y / 2.0f };

    rlPushMatrix();
    rlTranslatef(center.x, center.y, 0.0f);
    rlRotatef(rotation, 0, 0, 1);
    rlTranslatef(-center.x, -center.y, 0.0f);

    Rectangle rect = { position.x, position.y, size.x, size.y };
    DrawRectangleLinesEx(rect, thickness, color);

    rlPopMatrix();
}

std::weak_ptr<rbxInstance> clickable_instance;
std::weak_ptr<rbxInstance> next_clickable_instance;

std::weak_ptr<rbxInstance> topmost_instance;
std::weak_ptr<rbxInstance> next_topmost_instance;

std::weak_ptr<rbxInstance> getClickableGuiObject() {
    return clickable_instance;
}

std::weak_ptr<rbxInstance> getTopMostGuiObject() {
    return topmost_instance;
}

std::vector<std::weak_ptr<rbxInstance>> gui_objects_hovered;
std::vector<std::weak_ptr<rbxInstance>> next_gui_objects_hovered;

std::vector<std::weak_ptr<rbxInstance>> getGuiObjectsHovered() {
    return gui_objects_hovered;
}

std::vector<std::shared_ptr<rbxInstance>> mouse_enter_list;
std::vector<std::shared_ptr<rbxInstance>> mouse_leave_list;
std::map<rbxInstance*, bool> mouse_over_map;
std::map<std::shared_ptr<rbxInstance>, Rectangle> bounds_map;

// NOTE: expects Parent
bool isStorageChild(std::shared_ptr<rbxInstance> instance) {
    const auto parent = getInstanceValue<std::shared_ptr<rbxInstance>>(instance, PROP_INSTANCE_PARENT);
    assert(parent);

    return std::find(gui_storage_list.begin(), gui_storage_list.end(), parent) != gui_storage_list.end();
}
Vector2 Vector2Zero{0, 0};

struct GuiObjectBorder {
    Vector2 position;
    Vector2 size;
    float rotation;
    int border_size;
    Color border_color;
};

void renderText(std::shared_ptr<rbxInstance> instance, const char* text, Vector2 position, Vector2 rectangle_size, Color color, bool caret = false) {
    auto font = getInstanceValue<EngineFont>(instance, "FontFace").font;
    if (!font)
        return;

    auto size = getInstanceValue<float>(instance, "TextSize");

    Vector2 measured = MeasureTextEx(*font, text, size, 1.f);
    auto xalignment = getInstanceValue<EnumItem*>(instance, "TextXAlignment");
    auto yalignment = getInstanceValue<EnumItem*>(instance, "TextYAlignment");

    switch (xalignment->value) {
        case 0: // Left
            break;
        case 1: // Right
            position.x += rectangle_size.x - measured.x;
            break;
        case 2: // Center
            position.x += rectangle_size.x / 2.f - measured.x / 2.f;
            break;
    }
    switch (yalignment->value) {
        case 0: // Top
            break;
        case 1: // Center
            position.y += rectangle_size.y / 2.f - measured.y / 2.f;
            break;
        case 2: // Bottom
            position.y += rectangle_size.y / 2.f;
            break;
    }

    DrawTextEx(*font, text, position, size, 1.f, color);

    if (caret) {
        char* text_writable = const_cast<char*>(text);
        int cursor_pos = getInstanceValue<int>(instance, "CursorPosition") - 1;

        char old_ch = text_writable[cursor_pos];
        text_writable[cursor_pos] = '\0';

        Vector2 cursor_measured = MeasureTextEx(*font, text, size, 1.f);

        text_writable[cursor_pos] = old_ch;

        DrawLine(position.x + cursor_measured.x, position.y, position.x + cursor_measured.x, position.y + measured.y, DARKGRAY);
    }
}

void renderGuiObject(lua_State* L, std::shared_ptr<rbxInstance> instance, Vector2 mouse, Vector2 position_offset, bool anyImGui) {
    bool clips_descendants = false;
    std::optional<GuiObjectBorder> border_opt;

    const bool is_layer_collector = instance->isA("LayerCollector");

    // FIXME: we need to do something about descendants. currently, rotation has no effect. also, clips descendants doesn't do anything.
    // I think we need to render to individual render textures IN REVERSE ORDER?

    if (!is_layer_collector) {
        // FIXME: determine if this behavior is accurate; im trying to fix other issues right now and can't be bothered to verify this
        if (!instance->isA("GuiObject"))
            return;

        clips_descendants = getInstanceValue<bool>(instance, "ClipsDescendants");

        const auto parent = getInstanceValue<std::shared_ptr<rbxInstance>>(instance, PROP_INSTANCE_PARENT);
        const bool is_storage_child = isStorageChild(parent);

        // FIXME: separate function for pos&size calculations that only get called on parent changed?
        // necessary so a newly-created guiobject's Absolute* values are accurate before render
        auto parent_absolute_position = is_storage_child ? position_offset : getInstanceValue<Vector2>(parent, "AbsolutePosition");
        auto parent_absolute_size = is_storage_child ? rbxCamera::screen_size : getInstanceValue<Vector2>(parent, "AbsoluteSize");
        // auto parent_absolute_rotation = is_storage_child ? 0.0f : getInstanceValue<float>(parent, "AbsoluteRotation");

        auto& position = getInstanceValue<UDim2>(instance, "Position");
        auto& size = getInstanceValue<UDim2>(instance, "Size");
        float rotation = getInstanceValue<float>(instance, "Rotation");

        auto position_x_scale = position.x.scale;
        auto position_y_scale = position.y.scale;

        Vector2 absolute_position{
            parent_absolute_position.x + parent_absolute_size.x * position_x_scale + position.x.offset,
            parent_absolute_position.y + parent_absolute_size.y * position_y_scale + position.y.offset
        };
        Vector2 absolute_size{
            parent_absolute_size.x * size.x.scale + size.x.offset,
            parent_absolute_size.y * size.y.scale + size.y.offset
        };

        bounds_map[instance] = Rectangle{
            .x = absolute_position.x,
            .y = absolute_position.y,
            .width = absolute_size.x,
            .height = absolute_size.y
        };

        // float absolute_rotation = parent_absolute_rotation + rotation;
        float absolute_rotation = rotation;

        setInstanceValue<Vector2>(instance, L, "AbsolutePosition", absolute_position);
        setInstanceValue<Vector2>(instance, L, "AbsoluteSize", absolute_size);
        setInstanceValue<float>(instance, L, "AbsoluteRotation", absolute_rotation);

        if (!getInstanceValue<bool>(instance, "Visible"))
            return;

        auto background_color = getInstanceValue<Color>(instance, "BackgroundColor3");
        {
            auto position = auto_button_color_map.find(instance.get());
            if (position != auto_button_color_map.end() && position->second) {
                // TODO: hopefully optimize this
                const auto& hsv = ColorToHSV(background_color);
                background_color = ColorFromHSV(hsv.x, hsv.y, hsv.z / AUTO_BUTTON_COLOR_V);
            }
        }

        background_color.a = (1 - getInstanceValue<float>(instance, "BackgroundTransparency")) * 255;

        Rectangle shape_rect{
            .x = absolute_position.x + absolute_size.x / 2.f,
            .y = absolute_position.y + absolute_size.y / 2.f,
            .width = absolute_size.x,
            .height = absolute_size.y,
        };
        Vector2 shape_origin{absolute_size.x / 2.f, absolute_size.y / 2.f};

        DrawRectanglePro(shape_rect, shape_origin, absolute_rotation, background_color);
        // DrawRectanglePro(shape_rect, shape_origin, 0.f, background_color);

        auto border_size = getInstanceValue<int>(instance, "BorderSizePixel");
        if (border_size) {
            auto border_color = getInstanceValue<Color>(instance, "BorderColor3");
            border_color.a = background_color.a;

            if (clips_descendants)
                border_opt = GuiObjectBorder{ .position = absolute_position, .size = absolute_size, .rotation = absolute_rotation, .border_size = border_size, .border_color = border_color };
            else
                DrawRotatedRectangleLines(absolute_position, absolute_size, absolute_rotation, border_size, border_color);
                // DrawRotatedRectangleLines(absolute_position, absolute_size, 0.f, border_size, border_color);
        }

        const bool is_textbox = instance->isA("TextBox");

        bool text_rendered = false;
        if (instance->isA("TextLabel") || instance->isA("TextButton") || is_textbox) {
            auto& text = getInstanceValue<std::string>(instance, "Text");
            if (!text.empty()) {
                text_rendered = true; // NOTE: yes the text might not be rendered due to an invalid font, but this variable is used to render PlaceholderText so this is intended
                renderText(instance, text.c_str(), absolute_position, absolute_size, getInstanceValue<Color>(instance, "TextColor3"), is_textbox && UserInputService::isTextBoxFocused(instance));
            }
        }

        if (is_textbox && !text_rendered) {
            auto& text = getInstanceValue<std::string>(instance, "PlaceholderText");
            if (!text.empty())
                renderText(instance, text.c_str(), absolute_position, absolute_size, getInstanceValue<Color>(instance, "PlaceholderColor3"));
        }

        auto shape_lines = getRectangleLinesPro(shape_rect, shape_origin, absolute_rotation);
        const bool is_mouse_over = !anyImGui && CheckCollisionPointPoly(mouse, shape_lines.data(), shape_lines.size());

        auto& mouse_over_state = mouse_over_map[instance.get()];

        if (is_mouse_over) {
            next_gui_objects_hovered.push_back(instance);
            if (instance->isA("GuiButton"))
                next_clickable_instance = instance;
            next_topmost_instance = instance;
        }

        if (is_mouse_over != mouse_over_state)
            (is_mouse_over ? mouse_enter_list : mouse_leave_list).push_back(instance);

        mouse_over_state = is_mouse_over;
    }

    const auto child_count = instance->children.size();

    if (child_count) {
        if (clips_descendants)
            // FIXME: this needs to happen AFTER EACH CHILD'S BeginTextureMode, so these things should probably be arguments to this function
            // BeginScissorMode(clip_position.x, clip_position.y, clip_size.x, clip_size.y);
            ;

        // std::lock_guard lock(instance->children_mutex);

        std::vector<std::shared_ptr<rbxInstance>> sorted_children;

        sorted_children.reserve(child_count);

        sorted_children.insert(
            sorted_children.end(),
            instance->children.begin(),
            instance->children.end()
        );

        // TODO: use a std::set if stable sort is still possible (see drawingimmediate)
        std::stable_sort(sorted_children.begin(), sorted_children.end(), [] (std::shared_ptr<rbxInstance> a, std::shared_ptr<rbxInstance> b) {
            if (a->isA("GuiObject") && b->isA("GuiObject"))
                return getInstanceValue<int>(a, "ZIndex") < getInstanceValue<int>(b, "ZIndex");
            return false;
        });

        Vector2 position_offset = gui_inset_topleft;
        if (instance->isA("ScreenGui") && instance->getValue<bool>("IgnoreGuiInset"))
            position_offset = Vector2{0, menu_bar_height};

        for (size_t i = 0; i < sorted_children.size(); i++)
            renderGuiObject(L, sorted_children[i], mouse, position_offset, anyImGui);

        if (clips_descendants) {
            // FIXME: see above
            // EndScissorMode();
            if (border_opt.has_value()) {
                DrawRotatedRectangleLines(border_opt->position, border_opt->size, border_opt->rotation, border_opt->border_size, border_opt->border_color);
                // DrawRotatedRectangleLines(border_opt->position, border_opt->size, 0.f, border_opt->border_size, border_opt->border_color);
            }
        }
    }
}

std::vector<std::shared_ptr<rbxInstance>> render_list;
void contributeToRenderList(std::shared_ptr<rbxInstance> instance, bool is_storage = false) {
    // FIXME: verify child LayerCollector behavior in terms of DisplayOrder sorting. (if 'a' has higher DisplayOrder than 'b', but a layercollector 'c' parented to 'a' has a lower DisplayOrder than 'b', what happens?)

    if (!is_storage) {
        if (!instance->isA("LayerCollector"))
            return;
        if (!getInstanceValue<bool>(instance, "Enabled"))
            return;

        render_list.push_back(instance);
    }

    // std::lock_guard lock(instance->children_mutex);
    for (size_t i = 0; i < instance->children.size(); i++)
        contributeToRenderList(instance->children[i]);
}

void fireMouseMovementSignal(lua_State* L, Vector2& mouse, std::shared_ptr<rbxInstance> instance, const char* event) {
    pushFunctionFromLookup(L, fireRBXScriptSignal);
    instance->pushSignal(L, event, true);

    lua_pushnumber(L, mouse.x);
    lua_pushnumber(L, mouse.y);

    lua_call(L, 3, 0);
}

void rbxInstance_BasePlayerGui_render(lua_State *L) {
    next_clickable_instance.reset();
    next_topmost_instance.reset();
    next_gui_objects_hovered.clear();

    mouse_enter_list.clear();
    mouse_leave_list.clear();

    bounds_map.clear();

    // generate render list

    render_list.clear();

    for (size_t i = 0; i < gui_storage_list.size(); i++)
        contributeToRenderList(gui_storage_list[i], true);

    // TODO: use a std::set if stable sort is still possible (see drawingimmediate)
    std::stable_sort(render_list.begin(), render_list.end(), [] (std::shared_ptr<rbxInstance> a, std::shared_ptr<rbxInstance> b) {
        return getInstanceValue<int>(a, "DisplayOrder") < getInstanceValue<int>(b, "DisplayOrder");
    });

    auto mouse = GetMousePosition();

    // render objects
    for (size_t i = 0; i < render_list.size(); i++)
        renderGuiObject(L, render_list[i], mouse, Vector2Zero, UserInputService::any_imgui);

    clickable_instance = next_clickable_instance;
    topmost_instance = next_topmost_instance;
    gui_objects_hovered = next_gui_objects_hovered;

    // handle some signals (the rest are handled in UserInputService)

    for (size_t i = 0; i < mouse_enter_list.size(); i++) {
        auto& instance = mouse_enter_list[i];
        fireMouseMovementSignal(L, mouse, instance, "MouseEnter");
        UserInputService::signalMouseMovement(instance, InputBegan);
        handleGuiButtonMouseEnter(instance);
    }

    for (size_t i = 0; i < mouse_leave_list.size(); i++) {
        auto& instance = mouse_leave_list[i];
        fireMouseMovementSignal(L, mouse, instance, "MouseLeave");
        UserInputService::signalMouseMovement(instance, InputEnded);
        handleGuiButtonMouseLeave(instance);
    }
}

namespace rbxInstance_BasePlayerGui_methods {
    static int getGuiObjectsAtPosition(lua_State* L) {
        lua_checkinstance(L, 1, "BasePlayerGui");

        Vector2 position { 
            static_cast<float>(luaL_checknumber(L, 2)),
            static_cast<float>(luaL_checknumber(L, 3)),
        };

        static std::vector<std::pair<std::shared_ptr<rbxInstance>, Rectangle>> list;
        list.clear();
        list.insert(list.begin(), bounds_map.begin(), bounds_map.end());

        std::reverse(list.begin(), list.end());

        list.erase(std::remove_if(list.begin(), list.end(), [&position] (auto& a) {
            return !CheckCollisionPointRec(position, a.second);
        }));

        lua_createtable(L, list.size(), 0);

        for (size_t i = 0; i < list.size(); i++) {
            lua_pushinstance(L, list[i].first);
            lua_rawseti(L, -2, i + 1);
        }

        return 1;
    }
}; // namespace rbxInstance_BasePlayerGui_methods


void rbxInstance_BasePlayerGui_addStorageList(std::initializer_list<std::shared_ptr<rbxInstance>> initial_gui_storage_list) {
    gui_storage_list.insert(gui_storage_list.end(), initial_gui_storage_list.begin(), initial_gui_storage_list.end());
}

void rbxInstance_BasePlayerGui_init(lua_State *L) {
    rbxClass::class_map["BasePlayerGui"]->methods["GetGuiObjectsAtPosition"].func = rbxInstance_BasePlayerGui_methods::getGuiObjectsAtPosition;
}

};
