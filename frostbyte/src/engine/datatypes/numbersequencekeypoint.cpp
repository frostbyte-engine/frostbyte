#include "engine/datatypes/numbersequencekeypoint.hpp"

#include "common.hpp"
#include "userdata.hpp"

#include "lua.h"
#include "lualib.h"

namespace frostbyte {

int pushNumberSequenceKeypoint(lua_State* L, float envelope, float time, float value) {
    NumberSequenceKeypoint* numbersequencekeypoint = static_cast<NumberSequenceKeypoint*>(lua_newuserdatatagged(L, sizeof(NumberSequenceKeypoint), userdata::NumberSequenceKeypoint));
    numbersequencekeypoint->envelope = envelope;
    numbersequencekeypoint->time = time;
    numbersequencekeypoint->value = value;

    userdata::getClassMetatable(L, userdata::NumberSequenceKeypoint);
    lua_setmetatable(L, -2);

    return 1;
}
int pushNumberSequenceKeypoint(lua_State* L, NumberSequenceKeypoint numbersequencekeypoint) {
    return pushNumberSequenceKeypoint(L, numbersequencekeypoint.envelope, numbersequencekeypoint.time, numbersequencekeypoint.value);
}

static int NumberSequenceKeypoint_new(lua_State* L) {
    const double time = luaL_checknumber(L, 1);
    const double value = luaL_checknumber(L, 2);
    double envelope = luaL_optnumberloose(L, 3, 0.0);

    return pushNumberSequenceKeypoint(L, envelope, time, value);
}

bool lua_isnumbersequencekeypoint(lua_State* L, int index) {
    return userdata::is(L, index, userdata::NumberSequenceKeypoint);
}
NumberSequenceKeypoint* lua_checknumbersequencekeypoint(lua_State* L, int index) {
    void* ud = userdata::check(L, index, userdata::NumberSequenceKeypoint);

    return static_cast<NumberSequenceKeypoint*>(ud);
}

static int NumberSequenceKeypoint__tostring(lua_State* L) {
    NumberSequenceKeypoint* numbersequencekeypoint = lua_checknumbersequencekeypoint(L, 1);

    lua_pushfstringL(L, "%.*f %.*f %.*f", decimalFmt(numbersequencekeypoint->time), decimalFmt(numbersequencekeypoint->value), decimalFmt(numbersequencekeypoint->envelope));
    return 1;
}

static int NumberSequenceKeypoint__index(lua_State* L) {
    NumberSequenceKeypoint* numbersequencekeypoint = lua_checknumbersequencekeypoint(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strequal(key, "Envelope"))
        lua_pushnumber(L, numbersequencekeypoint->envelope);
    else if (strequal(key, "Time"))
        lua_pushnumber(L, numbersequencekeypoint->time);
    else if (strequal(key, "Value"))
        lua_pushnumber(L, numbersequencekeypoint->value);
    else
        goto INVALID;

    return 1;

    INVALID:
    luaL_error(L, "%s is not a valid member of NumberSequenceKeypoint", key);
}
static int NumberSequenceKeypoint__newindex(lua_State* L) {
    lua_checknumbersequencekeypoint(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strequal(key, "Envelope") || strequal(key, "Time") || strequal(key, "Value"))
        luaL_error(L, "%s member of NumberSequenceKeypoint is read-only and cannot be assigned to", key);

    luaL_error(L, "%s is not a valid member of NumberSequenceKeypoint", key);

    return 0;
}

void open_numbersequencekeypointlib(lua_State *L) {
    // NumberSequenceKeypoint
    lua_newtable(L);

    setfunctionfield(L, NumberSequenceKeypoint_new, "new", true);

    lua_setglobal(L, "NumberSequenceKeypoint");

    // metatable
    userdata::newClassMetatable(L, userdata::NumberSequenceKeypoint);
    setfunctionfield(L, NumberSequenceKeypoint__tostring, "__tostring");
    setfunctionfield(L, NumberSequenceKeypoint__index, "__index");
    setfunctionfield(L, NumberSequenceKeypoint__newindex, "__newindex");

    lua_pop(L, 1);
}

}; // namespace frostbyte
