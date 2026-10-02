#pragma once

#include "lua.h"

#include "engine/datatypes/udim2.hpp"

namespace frostbyte {

struct Path2DControlPoint {
    UDim2 position;
    UDim2 left_tangent;
    UDim2 right_tangent;
};

int pushPath2DControlPoint(lua_State* L, UDim2 position, UDim2 left_tangent, UDim2 right_tangent);
int pushPath2DControlPoint(lua_State* L, UDim2 position);

bool lua_ispath2dcontrolpoint(lua_State* L, int index);
Path2DControlPoint* lua_checkpath2dcontrolpoint(lua_State* L, int index);

void open_path2dcontrolpointlib(lua_State* L);

}; // namespace frostbyte

