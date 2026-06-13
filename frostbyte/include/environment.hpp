#pragma once

#include "lua.h"

namespace frostbyte {

#define env_expose(func) {                   \
    lua_pushcfunction(L, fr_##func, #func);  \
    lua_setglobal(L, #func);                 \
}
#define env_alias(func, alias) {                                          \
    lua_getglobal(L, #func);                                              \
    if (lua_type(L, -1) != LUA_TFUNCTION)                                 \
        luaL_error(L, "invalid environment index for alias: %s", #func);  \
    lua_setglobal(L, #alias);                                             \
}

int fr_getreg(lua_State* L);

int fr_getrawmetatable(lua_State* L);
int fr_setrawmetatable(lua_State* L);

void open_frostbyte_environment(lua_State* L);

}; // namespace frostbyte
