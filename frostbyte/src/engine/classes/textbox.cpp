#include "engine/classes/textbox.hpp"
#include "engine/classes/instance.hpp"
#include "engine/classes/userinputservice.hpp"

#include "lua.h"

namespace frostbyte {

namespace rbxInstance_TextBox_methods {
    static int captureFocus(lua_State* L) {
        auto textbox = lua_checkinstance(L, 1, "TextBox");

        UserInputService::captureTextBoxFocus(L, textbox);

        return 0;
    }
    static int isFocused(lua_State* L) {
        auto textbox = lua_checkinstance(L, 1, "TextBox");

        lua_pushboolean(L, UserInputService::isTextBoxFocused(textbox));
        return 1;
    }
    static int releaseFocus(lua_State* L) {
        auto textbox = lua_checkinstance(L, 1, "TextBox");

        UserInputService::releaseTextBoxFocus(L, textbox, false);

        return 0;
    }
}; // namespace rbxInstance_TextBox_methods

void rbxInstance_TextBox_init() {
    auto& this_class = rbxClass::class_map["TextBox"];

    this_class->methods["CaptureFocus"].func = rbxInstance_TextBox_methods::captureFocus;
    this_class->methods["IsFocused"].func = rbxInstance_TextBox_methods::isFocused;
    this_class->methods["ReleaseFocus"].func = rbxInstance_TextBox_methods::releaseFocus;
}

}; // namespace frostbyte
