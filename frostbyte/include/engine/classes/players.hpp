#pragma once

#include "engine/classes/instance.hpp"

namespace frostbyte {

class Players {
public:
};

void rbxInstance_Players_init(lua_State* L, std::shared_ptr<rbxInstance> datamodel);

}; // namespace frostbyte
