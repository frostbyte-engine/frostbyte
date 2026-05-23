#pragma once

#include "engine/datatypes/enum.hpp"

#include "raylib.h"

#include "lua.h"

namespace frostbyte {

struct EngineFont {
    std::string family;
    EnumItem* weight;
    EnumItem* style;
    Font* font;
};

int pushFont(lua_State* L, std::string family, EnumItem* weight, EnumItem* style);
int pushFont(lua_State* L, EngineFont engine_font);
EngineFont* lua_checkfont(lua_State* L, int narg);

void open_fontlib(lua_State* L);

}; // namespace frostbyte
