#include "engine/datatypes/rbxscriptsignal.hpp"
#include "engine/datatypes/rbxscriptconnection.hpp"

#include "common.hpp"
#include "taskscheduler.hpp"
#include "userdata.hpp"

#include "lua.h"
#include "lualib.h"

#include <algorithm>
#include <unordered_map>

namespace frostbyte {

std::vector<rbxScriptSignal*> signal_list;
std::unordered_map<rbxScriptSignal*, size_t> signal_list_index_map;

rbxScriptSignal::~rbxScriptSignal() {
    auto it = signal_list_index_map.find(this);
    if (it == signal_list_index_map.end()) return;

    size_t index = it->second;
    rbxScriptSignal*& last = signal_list.back();

    signal_list_index_map[last] = index;
    signal_list[index] = std::move(last);
    signal_list.pop_back();
    signal_list_index_map.erase(it);
}

void rbxScriptSignal::cleanup() {
    while (!signal_list.empty())
        signal_list.back()->~rbxScriptSignal();
}

int pushNewRBXScriptSignal(lua_State* L, const char* name) {
    lua_getfield(L, LUA_REGISTRYINDEX, SIGNALCONNECTIONLISTLOOKUP);

    rbxScriptSignal* signal = static_cast<rbxScriptSignal*>(lua_newuserdatatagged(L, sizeof(rbxScriptSignal), userdata::RBXScriptSignal));
    new(signal) rbxScriptSignal();

    signal_list_index_map[signal] = signal_list.size();
    signal_list.push_back(signal);

    signal->name.assign(name);

    userdata::getClassMetatable(L, userdata::RBXScriptSignal);
    lua_setmetatable(L, -2);

    lua_pushvalue(L, -1);
    lua_newtable(L);
    lua_rawset(L, -4);

    lua_remove(L, -2);

    return 1;
}

rbxScriptSignal* lua_checkrbxscriptsignal(lua_State* L, int narg) {
    void* ud = userdata::check(L, narg, userdata::RBXScriptSignal);

    return static_cast<rbxScriptSignal*>(ud);
}

void pushSignalConnectionList(lua_State* L, int narg) {
    if (narg < 0 && narg > LUA_REGISTRYINDEX)
        narg--;

    lua_getfield(L, LUA_REGISTRYINDEX, SIGNALCONNECTIONLISTLOOKUP);
    lua_pushvalue(L, narg);
    lua_rawget(L, -2);
    lua_remove(L, -2);
}

static int wait_proxy(lua_State* L) {
    const int thread_index = lua_upvalueindex(1);
    const int connection_index = lua_upvalueindex(2);

    luaL_checktype(L, thread_index, LUA_TTHREAD);
    lua_State* thread = lua_tothread(L, thread_index);

    auto connection = lua_checkrbxscriptconnection(L, connection_index);
    connection->destroy(L);

    const int arg_count = lua_gettop(L);
    lua_xmove(L, thread, arg_count);

    TaskScheduler::queueForResume(thread, arg_count);

    return 0;
}
namespace rbxScriptSignal_methods {
    static int connect(lua_State* L) {
        lua_checkrbxscriptsignal(L, 1);
        luaL_checktype(L, 2, LUA_TFUNCTION);

        pushNewRBXScriptConnection(L, 2);

        pushSignalConnectionList(L, 1);
        lua_pushvalue(L, -2);

        lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
        lua_pop(L, 1);

        return 1;
    }
    static int wait(lua_State* L) {
        lua_checkrbxscriptsignal(L, 1);

        pushSignalConnectionList(L, 1);

        pushNewRBXScriptConnection(L, [&L]() {
            lua_pushthread(L);
            lua_pushvalue(L, -3); // connection
            lua_pushcclosure(L, wait_proxy, "wait_proxy", 2);
        });

        lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
        lua_pop(L, 1);

        return TaskScheduler::yieldThread(L);
    }
};
lua_CFunction getrbxScriptSignalMethod(const char* key) {
    // TODO: Parallel Luau
    if (strequal(key, "Connect") || strequal(key, "connect") ||
        strequal(key, "ConnectParallel") || strequal(key, "connectParallel")
    )
        return rbxScriptSignal_methods::connect;
    else if (strequal(key, "Wait") || strequal(key, "wait"))
        return rbxScriptSignal_methods::wait;

    return nullptr;
}

static int rbxScriptSignal__tostring(lua_State* L) {
    rbxScriptSignal* signal = lua_checkrbxscriptsignal(L, 1);

    lua_pushfstringL(L, "Signal %s", signal->name.c_str());
    return 1;
}

static int rbxScriptSignal__index(lua_State* L) {
    lua_checkrbxscriptsignal(L, 1);
    const char* key = lua_tostring(L, 2);

    lua_CFunction func = getrbxScriptSignalMethod(key);
    if (!func)
        luaL_error(L, "%s is not a valid member of RBXScriptSignal", key);

    return pushFunctionFromLookup(L, func, key);
}
static int rbxScriptSignal__newindex(lua_State* L) {
    lua_checkrbxscriptsignal(L, 1);
    const char* key = lua_tostring(L, 2);

    luaL_error(L, "%s is not a valid member of RBXScriptSignal", key);
    return 0;
}
static int rbxScriptSignal__namecall(lua_State* L) {
    lua_checkrbxscriptsignal(L, 1);
    const char* namecall = lua_namecallatom(L, nullptr);
    if (!namecall)
        luaL_error(L, "no namecall method!");

    lua_CFunction func = getrbxScriptSignalMethod(namecall);
    if (!func)
        luaL_error(L, "%s is not a valid member of RBXScriptSignal", namecall);

    return func(L);
}

int fireRBXScriptSignalWithFilter(lua_State* L) {
    lua_checkrbxscriptsignal(L, 1);
    bool use_filter = !lua_isnil(L, 2);
    if (use_filter)
        luaL_checktype(L, 2, LUA_TFUNCTION);

    int arg_count = lua_gettop(L) - 2;

    pushFunctionFromLookup(L, fr_task_spawn, "spawn");

    pushSignalConnectionList(L, 1);
    lua_remove(L, 1);

    int count = lua_objlen(L, -1);
    for (int i = count; i > 0; i--) {
        lua_rawgeti(L, -1, i);
        lua_pushvalue(L, -3); // task.spawn
        lua_pushvalue(L, -2); // connection

        rbxScriptConnection* connection = lua_checkrbxscriptconnection(L, -1);
        if (!connection->alive) {
            lua_pop(L, 3);
            continue;
        }

        int function_index = connection->function_index;
        lua_remove(L, -1);

        lua_getfield(L, LUA_REGISTRYINDEX, RBXSCRIPTCONNECTION_METHODLOOKUP);
        lua_rawgeti(L, -1, function_index); // function
        lua_remove(L, -2);

        if (use_filter) {
            lua_pushvalue(L, 1); // filter
            lua_pushvalue(L, -2); // function
            lua_call(L, 1, 1);

            bool skip = luaL_optboolean(L, -1, false);
            lua_pop(L, 1);

            if (skip) {
                lua_pop(L, 3);
                continue;
            }
        }

        for (int i = 0; i < arg_count; i++)
            lua_pushvalue(L, 2 + i);

        lua_call(L, arg_count + 1, 0);

        lua_pop(L, 1);
    }

    return 0;
}
int fireRBXScriptSignal(lua_State* L) {
    lua_pushnil(L);
    lua_insert(L, 2);

    return fireRBXScriptSignalWithFilter(L);
}

int disconnectAllRBXScriptSignal(lua_State *L) {
    lua_checkrbxscriptsignal(L, 1);

    pushSignalConnectionList(L, 1);
    lua_remove(L, 1);

    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        rbxScriptConnection* connection = lua_checkrbxscriptconnection(L, -1);
        connection->destroy(L);

        lua_pop(L, 1);
    }

    return 0;
}

void setup_rbxScriptSignal(lua_State *L) {
    // signallookup
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, SIGNALLOOKUP);

    // connection list lookup
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, SIGNALCONNECTIONLISTLOOKUP);

    // metatable
    userdata::newClassMetatable(L, userdata::RBXScriptSignal);
    setfunctionfield(L, rbxScriptSignal__tostring, "__tostring");
    setfunctionfield(L, rbxScriptSignal__index, "__index");
    setfunctionfield(L, rbxScriptSignal__newindex, "__newindex");
    setfunctionfield(L, rbxScriptSignal__namecall, "__namecall");

    lua_pop(L, 1);

    setup_rbxScriptConnection(L);
}

}; // namespace frostbyte
