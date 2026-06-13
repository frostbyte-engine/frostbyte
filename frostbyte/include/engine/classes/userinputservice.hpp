#pragma once

#include "engine/classes/instance.hpp"

#include "lua.h"

namespace frostbyte {

enum InputState {
    InputBegan,
    InputChanged,
    InputEnded
};

class UserInputService {
public:
    static bool is_window_focused;
    static bool any_imgui;
    static Vector2 mouse_position;
    static Vector2 mouse_delta;

    static void signalMouseMovement(std::shared_ptr<rbxInstance> instance, InputState type);
    static void releaseTextBoxFocus(lua_State* L, std::shared_ptr<rbxInstance> textbox, bool enter_pressed);
    static void captureTextBoxFocus(lua_State* L, std::shared_ptr<rbxInstance> textbox);
    static bool isTextBoxFocused(std::shared_ptr<rbxInstance> textbox);

    static void process(lua_State* L);
};

void rbxInstance_UserInputService_init();

}; // namespace frostbyte
