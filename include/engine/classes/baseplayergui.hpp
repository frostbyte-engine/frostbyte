#pragma once

#include "engine/classes/instance.hpp"

#include "lua.h"

#include <initializer_list>

namespace frostbyte {

std::weak_ptr<rbxInstance> getClickableGuiObject();
std::weak_ptr<rbxInstance> getTopMostGuiObject();
std::vector<std::weak_ptr<rbxInstance>> getGuiObjectsHovered();

void rbxInstance_BasePlayerGui_render(lua_State* L);
void rbxInstance_BasePlayerGui_addStorageList(std::initializer_list<std::shared_ptr<rbxInstance>> initial_gui_storage_list);

void rbxInstance_BasePlayerGui_init(lua_State* L);

};
