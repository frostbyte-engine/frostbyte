#pragma once

#include "engine/datatypes/enum.hpp"

namespace frostbyte {

struct TweenInfo {
    EnumItem* easing_direction = nullptr;
    EnumItem* easing_style = nullptr;
    double time = 1;
    double delay_time = 0;
    int repeat_count = 0;
    bool reverses = false;
};

int pushTweenInfo(lua_State* L, TweenInfo tweeninfo);
TweenInfo* lua_checktweeninfo(lua_State* L, int narg);

void open_tweeninfolib(lua_State* L);

}; // namespace frostbyte
