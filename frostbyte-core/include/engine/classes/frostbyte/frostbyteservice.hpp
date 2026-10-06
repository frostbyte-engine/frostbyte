#pragma once

#include "engine/classes/instance.hpp"

namespace frostbyte {

class FrostbyteService {
public:
    #ifndef FROSTBYTE_HEADLESS
    static std::shared_ptr<rbxInstance> menu_bar;
    #endif
    static std::shared_ptr<rbxInstance> options;
};

void FrostbyteService_init(lua_State *L, std::shared_ptr<rbxInstance> datamodel);

}; // namespace frostbyte

