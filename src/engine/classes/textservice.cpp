#include "engine/classes/textservice.hpp"
#include "engine/classes/instance.hpp"
#include "engine/datatypes/enum.hpp"
#include "engine/datatypes/font.hpp"
#include "engine/datatypes/vector2.hpp"

#include "lua.h"
#include "lualib.h"

#include "raylib.h"

namespace frostbyte {

namespace rbxInstance_TextService_methods {
static int getTextSize(lua_State* L) {
    lua_checkinstance(L, 1, "TextService");

    const char* text = luaL_checkstring(L, 2);
    double font_size = luaL_checknumber(L, 3);
    EnumItem* font_enum_item = lua_checkenumitem(L, 4, "Font");
    /*auto frame_size = */lua_checkvector2(L, 5);

    Font* font = getFontFromEnum(L, font_enum_item);
    if (!font)
        luaL_error(L, "[INTERNAL ERROR] failed to get font from enum item");

    // TODO: incorporate frame_size

    auto measured = MeasureTextEx(*font, text, font_size, 1.0f);

    return pushVector2(L, measured);
}
};

void rbxInstance_TextService_init() {
    rbxClass::class_map["TextService"]->methods["GetTextSize"].func = rbxInstance_TextService_methods::getTextSize;
}

}; // namespace frostbyte
