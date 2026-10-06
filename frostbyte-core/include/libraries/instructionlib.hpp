#pragma once

#include "frostbyteoption.hpp"

#include "lua.h"

namespace frostbyte {

extern FrostbyteOption enable_stephook;

void open_instructionlib(lua_State* L);

void onEnableStephookChange(lua_State* L);

}; // namespace frostbyte
