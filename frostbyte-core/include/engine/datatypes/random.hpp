#pragma once

#include "lua.h"

#include <cstdint>

namespace frostbyte {

struct Random {
    int64_t seed;
    uint64_t state = 0;

    Random(int64_t seed): seed(seed) {}

    uint32_t nextUint();
    int nextInteger(int min, int max);
    double nextDouble();
};

Random* lua_checkrandom(lua_State* L, int narg);

void open_randomlib(lua_State* L);

}; // namespace frostbyte
