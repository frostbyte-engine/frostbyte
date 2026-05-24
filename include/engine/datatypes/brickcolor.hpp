#pragma once

#include "lua.h"

#include "raylib.h"

#include <vector>

namespace frostbyte {

struct BrickColor {
    const char* name;
    int index;
    Color color;
};

extern std::vector<BrickColor> brick_color_list;

BrickColor* getBrickColor(const char* name);
BrickColor* getBrickColor(int index);
BrickColor* getBrickColor(Color color);

int pushBrickColor(lua_State* L, int index);
int pushBrickColor(lua_State* L, const char* name);
int pushBrickColor(lua_State* L, Color color);

bool lua_isbrickcolor(lua_State* L, int narg);
BrickColor* lua_checkbrickcolor(lua_State* L, int narg);

void open_brickcolorlib(lua_State* L);

}; // namespace frostbyte
