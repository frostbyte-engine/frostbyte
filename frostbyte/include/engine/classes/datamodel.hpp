#pragma once

#include "engine/classes/instance.hpp"

namespace frostbyte {

class DataModel {
public:
    static std::shared_ptr<rbxInstance> instance;

    static bool loaded;
    static bool shutdown;

    static void onLoad(lua_State* L);
    static void onShutdown(lua_State* L);
};

void rbxInstance_DataModel_init(lua_State* L);

}; // namespace frostbyte
