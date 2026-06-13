#pragma once

#include "lua.h"
#include "raylib.h"

namespace frostbyte {

int pushVector3(lua_State* L, float x, float y, float z);
int pushVector3(lua_State* L, Vector3 vector3);

bool lua_isvector3(lua_State* L, int narg);
Vector3* lua_checkvector3(lua_State* L, int narg);

void open_vector3lib(lua_State* L);

}; // namespace frostbyte
