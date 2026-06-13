#pragma once

#include "lua.h"

#include <cstdio>
#include <functional>

namespace frostbyte {

class rbxScriptConnection {
public:
    bool alive = true;
    bool once = false;
    int function_index;

    void destroy(lua_State* L);
};

int pushNewRBXScriptConnection(lua_State* L, std::function<void()> pushValue, bool once = false);
int pushNewRBXScriptConnection(lua_State* L, int func_index, bool once = false);
rbxScriptConnection* lua_checkrbxscriptconnection(lua_State* L, int narg);

void setup_rbxScriptConnection(lua_State* L);

};
