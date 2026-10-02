#pragma once

#include "engine/classes/instance.hpp"

namespace frostbyte {

class CoreGui {
public:
    static std::shared_ptr<rbxInstance> notification_frame;
};

void rbxInstance_CoreGui_setup(lua_State* L, std::shared_ptr<rbxInstance> instance);

}; // namespace frostbyte
