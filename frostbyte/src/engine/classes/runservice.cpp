#include "engine/classes/runservice.hpp"
#include "engine/datatypes/rbxscriptsignal.hpp"

#include "common.hpp"
#include "console.hpp"
#include "taskscheduler.hpp"
#include "ui/ui.hpp"

#include "lapi.h"
#include "ltable.h"
#include "lua.h"
#include "lualib.h"

namespace frostbyte {

std::shared_ptr<rbxInstance> RunService::instance;

// std::shared_mutex bind_list_mutex;

static int compare(lua_State* L) {
    lua_rawgeti(L, 1, 1);
    lua_rawgeti(L, 2, 1);

    lua_pushboolean(L, lua_tointeger(L, 3) < lua_tointeger(L, 4));
    return 1;
}

namespace rbxInstance_RunService_methods {
    static int bindToRenderStep(lua_State* L) {
        // std::lock_guard lock(bind_list_mutex);

        lua_checkinstance(L, 1, "RunService");
        const char* name = luaL_checkstring(L, 2);
        luaL_checkinteger(L, 3);
        luaL_checktype(L, 4, LUA_TFUNCTION);

        lua_getglobal(L, "table");
        lua_rawgetfield(L, -1, "sort");
        lua_remove(L, -2);

        lua_rawgetfield(L, LUA_REGISTRYINDEX, BINDLIST_KEY);

        lua_rawgetfield(L, -1, name);
        if (lua_isnil(L, -1)) {
            lua_pop(L, 1);
            lua_createtable(L, 0, 1);
            lua_pushvalue(L, -1);
            lua_rawsetfield(L, -3, name);
        }

        lua_createtable(L, 2, 0);

        lua_pushvalue(L, 3);
        lua_rawseti(L, -2, 1);

        lua_pushvalue(L, 4);
        lua_rawseti(L, -2, 2);

        lua_rawseti(L, -2, lua_objlen(L, -2) + 1);
        lua_remove(L, -2);
        // FIXME: do we need to keep a separate sorted list? based on Roblox docs, I am lead to believe that a lower priority entry should be ran before a higher priority entry on a universal, not local like it is currently, scale
        pushFunctionFromLookup(L, compare);

        lua_call(L, 2, 0);

        return 0;
    }

    static int isClient(lua_State* L) {
        lua_pushboolean(L, !runservice_is_server);
        return 1;
    }
    static int isServer(lua_State* L) {
        lua_pushboolean(L, runservice_is_server);
        return 1;
    }
    static int isStudio(lua_State* L) {
        lua_pushboolean(L, runservice_is_studio);
        return 1;
    }

    static int unbindFromRenderStep(lua_State* L) {
        // std::lock_guard lock(bind_list_mutex);

        lua_checkinstance(L, 1, "RunService");
        const char* name = luaL_checkstring(L, 2);

        lua_rawgetfield(L, LUA_REGISTRYINDEX, BINDLIST_KEY);
        lua_rawgetfield(L, -1, name);

        if (!lua_isnil(L, -1)) {
            const TValue* obj = luaA_toobject(L, -1);
            LUAU_ASSERT(obj->tt == LUA_TTABLE);

            LuaTable* tt = hvalue(obj);
            int count = luaH_getn(tt);
            luaH_clear(tt);

            if (count > 1)
                getTask(L)->console->warningf("RunService:UnbindFromRenderStep removed different functions with same reference name %s %d times.", name, count);
        }

        return 0;
    }
}; // namespace rbxInstance_RunService_methods

void fireEventWithDelta(lua_State* L, const char* event, std::shared_ptr<rbxInstance>& instance, double delta) {
    pushFunctionFromLookup(L, fireRBXScriptSignal);
    instance->pushSignal(L, event, true);
    lua_pushnumber(L, delta);

    lua_call(L, 2, 0);
}

double last_process = lua_clock();
void RunService::process(lua_State *L) {
    // std::lock_guard lock(bind_list_mutex);

    const double clock = lua_clock();
    const double delta = clock - last_process;
    last_process = clock;

    lua_rawgetfield(L, LUA_REGISTRYINDEX, BINDLIST_KEY);

    lua_pushnil(L);
    while (lua_next(L, -2)) {
        lua_pushnil(L);
        while (lua_next(L, -2)) {
            lua_rawgeti(L, -1, 2);
            lua_pushnumber(L, delta);

            // NOTE: in Roblox, if you task.wait unside the callback, there will be a normal error message.
            // this is like if we were to throw exception in startFunctionOnNewThread and handle error there but then not provide a feedback function
            TaskScheduler::startFunctionOnNewThread(L, [] (std::string error) {
                Console::ScriptConsole.errorf("%.*s", (int) error.size(), error.data());
                Console::ScriptConsole.errorf("RunService:fireRenderStepEarlyFunctions unexpected error while invoking callback: %.*s", (int) error.size(), error.data());
            }, 1);

            lua_pop(L, 1);
        }

        lua_pop(L, 1);
    }

    lua_pop(L, 1);

    fireEventWithDelta(L, "PreRender", instance, delta);
    fireEventWithDelta(L, "RenderStepped", instance, delta);
}
double last_heartbeat = lua_clock();
void RunService::heartbeat(lua_State *L) {
    const double clock = lua_clock();
    const double delta = clock - last_heartbeat;
    last_heartbeat = clock;

    fireEventWithDelta(L, "Heartbeat", instance, delta);
}

void rbxInstance_RunService_init(lua_State* L) {
    lua_newtable(L);
    lua_rawsetfield(L, LUA_REGISTRYINDEX, BINDLIST_KEY);

    rbxClass::class_map["RunService"]->methods["BindToRenderStep"].func = rbxInstance_RunService_methods::bindToRenderStep;
    rbxClass::class_map["RunService"]->methods["IsClient"].func = rbxInstance_RunService_methods::isClient;
    rbxClass::class_map["RunService"]->methods["IsServer"].func = rbxInstance_RunService_methods::isServer;
    rbxClass::class_map["RunService"]->methods["IsStudio"].func = rbxInstance_RunService_methods::isStudio;
    rbxClass::class_map["RunService"]->methods["UnbindFromRenderStep"].func = rbxInstance_RunService_methods::unbindFromRenderStep;
}

}; // namespace frostbyte
