#pragma once

#include "engine/classes/instance.hpp"
#include "lua.h"

namespace frostbyte {

void ImGuiService_init(lua_State *L, std::shared_ptr<rbxInstance> datamodel);

void ImGuiService_render(lua_State* L);

}; // namespace frostbyte
