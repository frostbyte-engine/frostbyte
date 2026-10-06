#pragma once

#include "lua.h"
#include <vector>

namespace frostbyte {

struct FrostbyteOptionUpdate {
    const char* name;
    bool value;
};
extern std::vector<FrostbyteOptionUpdate> option_update_list;

void FrostbyteOptions_init(lua_State* L);

}; // namespace frostbyte


