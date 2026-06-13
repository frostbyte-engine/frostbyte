#pragma once

#include "raylib.h"

#include "lua.h"

namespace frostbyte {

int pushColor(lua_State* L, float r, float g, float b);
int pushColor(lua_State* L, Color color);
bool lua_iscolor(lua_State* L, int narg);
Color* lua_checkcolor(lua_State* L, int narg);

void open_color3lib(lua_State* L);

}; // namespace frostbyte
