#include "environment.hpp"

#include "common.hpp"
#include "raylib.h"
#include "taskscheduler.hpp"

#include "engine/classes/userinputservice.hpp"
#include "engine/classes/runservice.hpp"
#include "engine/datatypes/rbxscriptsignal.hpp"

#include "libraries/drawentrylib.hpp"
#include "libraries/debuglib.hpp"

#include <chrono>
#include <string>

#include "Luau/Common.h"
#include "Luau/Compiler.h"
#include "lua.h"
#include "lualib.h"
#include "lgc.h"
#include "lapi.h"
#include "lmem.h"
#include "lobject.h"
#include "lstate.h"
#include "ltable.h"

namespace frostbyte {

static int fr_getexecutorname(lua_State* L) {
    lua_pushstring(L, "frostbyte");

    return 1;
}

static int fr_identifyexecutor(lua_State* L) {
    lua_pushstring(L, "frostbyte");
    // TODO: actual version?
    lua_pushstring(L, "v1");

    return 2;
}

static int fr_Version(lua_State* L) {
    // NOTE: this is the studio version from July 1 since we got the api dump from July 8
    lua_pushstring(L, "0.680.0.6800701");
    return 1;
}

int fr_getreg(lua_State* L) {
    lua_pushvalue(L, LUA_REGISTRYINDEX);
    return 1;
}
static int fr_getgc(lua_State* L) {
    TaskScheduler::performGCWork(L, [&L] {
        struct VisitUserdata_t {
            bool tables;
            std::vector<GCObject*> list;
        } VisitUserdata;
        VisitUserdata.tables = luaL_optboolean(L, 1, 0);

        luaM_visitgco(L, &VisitUserdata, [](void* ud, lua_Page* page, GCObject* object) {
            auto visit_ud = static_cast<VisitUserdata_t*>(ud);

            if (object->gch.tt == LUA_TTABLE && !visit_ud->tables)
                goto RET;

            switch (object->gch.tt) {
                case LUA_TTABLE:
                case LUA_TFUNCTION:
                case LUA_TUSERDATA:
                case LUA_TTHREAD:
                    break;
                default:
                    goto RET;
            }

            visit_ud->list.push_back(object);

            RET:
            return false;
        });

        createweaktable(L, VisitUserdata.list.size(), 0);

        for (size_t i = 0; i < VisitUserdata.list.size(); i++) {
            auto& obj = VisitUserdata.list[i];
            lua_pushnil(L);
            auto new_value = const_cast<TValue*>(luaA_toobject(L, -1));
            switch (obj->gch.tt) {
                case LUA_TTABLE:
                    sethvalue(L, new_value, gco2h(obj));
                    break;
                case LUA_TFUNCTION:
                    setclvalue(L, new_value, gco2cl(obj));
                    break;
                case LUA_TUSERDATA:
                    setuvalue(L, new_value, gco2u(obj));
                    break;
                case LUA_TTHREAD:
                    setthvalue(L, new_value, gco2th(obj));
                    break;
                default:
                    LUAU_UNREACHABLE();
            }
            lua_rawseti(L, -2, i + 1);
        }
    });

    return 1;
}

static int fr_getallthreads(lua_State* L) {
    // std::shared_lock lock(TaskScheduler::thread_list_mutex);

    createweaktable(L, TaskScheduler::thread_list.size(), 0);
    for (size_t i = 0; i < TaskScheduler::thread_list.size();i ++) {
        lua_State* thread = TaskScheduler::thread_list[i];
        lua_pushnil(L);
        setthvalue(L, L->top - 1, thread);
        lua_rawseti(L, -2, i + 1);
    }

    return 1;
}

// FIXME: this function is broken since I adjusted the bindlist to be a table of tables
static int fr_getrendersteppedlist(lua_State* L) {
    if (lua_gettop(L))
        luaL_error(L, "too many arguments to getrendersteppedlist! expected 0");

    lua_rawgetfield(L, LUA_REGISTRYINDEX, BINDLIST_KEY);
    luaL_checktype(L, 1, LUA_TTABLE);

    LuaTable* table = hvalue(luaA_toobject(L, 1));
    const int item_count = luaH_getn(table);

    createweaktable(L, item_count, 0);

    int i = 0;
    lua_pushnil(L);
    while (lua_next(L, 1)) {
        lua_rawgeti(L, -1, 2);
        lua_rawseti(L, 2, ++i);

        lua_pop(L, 1);
    }

    return 1;
}

static int fr_rawtostring(lua_State* L) {
    luaL_checkany(L, 1);
    std::string str = rawtostring(L, 1);

    lua_pushlstring(L, str.c_str(), str.size());
    return 1;
}

// from Luau/CLI/src/Repl.cpp
static int fr_loadstring(lua_State* L) {
    size_t l = 0;
    const char* s = luaL_checklstring(L, 1, &l);
    const char* chunkname = luaL_optstring(L, 2, "");

    lua_setsafeenv(L, LUA_ENVIRONINDEX, false);

    std::string bytecode = Luau::compile(std::string(s, l));
    if (luau_load(L, chunkname, bytecode.data(), bytecode.size(), 0) == 0)
        return 1;

    lua_pushnil(L);
    lua_insert(L, -2); // put before error message
    return 2;          // return nil plus error message
}

// a wait implementation that isn't built into the task scheduler because Roblox also has a deprecated wait global used before the task scheduler was introduced
// NOTE: the default value should be 0.03
static int fr_wait(lua_State* L) {
    const double seconds = getSeconds(L, 1);
    auto before = std::chrono::system_clock::now();
    auto end = std::chrono::system_clock::now() + std::chrono::duration<double>(seconds);

    return TaskScheduler::yieldForWork(L, [before, end] (Yield yield) {
        auto now = std::chrono::system_clock::now();
        while (now < end)
            now = std::chrono::system_clock::now();

        yield.finish([now, before] (lua_State* L) {
            lua_pushnumber(L, static_cast<std::chrono::duration<double>>(now - before).count());
            lua_pushnumber(L, lua_clock() - TaskScheduler::initial_client_time);
            return 2;
        });
    }, true);
}

static int fr_gcstep(lua_State* L) {
    lua_gc(L, LUA_GCSTEP, 0);
    return 0;
}
static int fr_gcfull(lua_State* L) {
    lua_gc(L, LUA_GCCOLLECT, 0);
    return 0;
}

static int fr_getnamecallmethod(lua_State* L) {
    const char* method = L->namecall ? getstr(L->namecall) : NULL;
    if (method)
        lua_pushstring(L, method);
    else
        luaL_error(L, "not in namecall context!");

    return 1;
}

static int fr_setnamecallmethod(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);

    if (!L->namecall)
        luaL_error(L, "not in namecall context!");

    L->namecall = tsvalue(luaA_toobject(L, 1));

    return 0;
}

static int fr_islclosure(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    Closure* cl = clvalue(luaA_toobject(L, 1));

    lua_pushboolean(L, !cl->isC);
    return 1;
}
static int fr_iscclosure(lua_State* L) {
    luaL_checktype(L, 1, LUA_TFUNCTION);
    Closure* cl = clvalue(luaA_toobject(L, 1));

    lua_pushboolean(L, cl->isC);
    return 1;
}

int fr_getrawmetatable(lua_State* L) {
    luaL_checkany(L, 1);

    if (!lua_getmetatable(L, 1))
        lua_pushnil(L);
    return 1;
}

int fr_setrawmetatable(lua_State* L) {
    if (lua_gettop(L) > 2)
        luaL_error(L, "too many arguments to setrawmetatable! expected 2");

    luaL_checkany(L, 1);
    luaL_argexpected(L, lua_isnil(L, 2) || lua_istable(L, 2), 2, "nil or table");

    lua_setmetatable(L, 1);
    return 0;
}

static int fr_setsafeenv(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    const bool enabled = luaL_checkboolean(L, 2);
    lua_setsafeenv(L, 1, enabled);

    return 0;
}
static int fr_issafeenv(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    LuaTable* t = hvalue(luaA_toobject(L, 1));

    lua_pushboolean(L, t->safeenv);
    return 1;
}

static int fr_trawfreeze(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_argcheck(L, !lua_getreadonly(L, 1), 1, "table is already frozen");

    lua_setreadonly(L, 1, true);

    lua_pushvalue(L, 1);
    return 1;
}
static int fr_trawunfreeze(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    luaL_argcheck(L, lua_getreadonly(L, 1), 1, "table is not frozen");

    lua_setreadonly(L, 1, false);

    lua_pushvalue(L, 1);
    return 1;
}
static int fr_tsetfrozen(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTABLE);
    const bool frozen = luaL_checkboolean(L, 2);

    lua_setreadonly(L, 1, frozen);

    lua_pushvalue(L, 1);
    return 1;
}

static int fr_iswindowactive(lua_State* L) {
    lua_pushboolean(L, UserInputService::is_window_focused);
    return 1;
}

static int fr_setfpscap(lua_State* L) {
    TaskScheduler::setTargetFps(luaL_checknumberrange(L, 1, 0, static_cast<unsigned>(-1), "target_fps"));

    return 0;
}
static int fr_getfpscap(lua_State* L) {
    lua_pushinteger(L, TaskScheduler::target_fps);
    return 1;
}

static int fr_setwindowtitle(lua_State* L) {
    SetWindowTitle(luaL_checkstring(L, 1));

    return 0;
}

static int fr_setclipboard(lua_State* L) {
    SetClipboardText(luaL_checkstring(L, 1));
    return 0;
}

static int fr_getclipboard(lua_State* L) {
    if (const char* text = GetClipboardText())
        lua_pushstring(L, text);
    else
        lua_pushstring(L, "");

    return 1;
}

static int fr_getgenv(lua_State* L) {
    if (TaskScheduler::sandboxing)
        luaL_error(L, "you cannot use getgenv while sandboxing is enabled! rerun frostbyte with the --nosandbox flag");

    // TODO: should this be environindex ?? i have no idea
    lua_pushvalue(L, LUA_GLOBALSINDEX);
    return 1;
}
static int fr_getrenv(lua_State* L) {
    // TODO: getrenv; should essentially be shared but with all environment values (__index = environment maybe); should be one specific table we keep in registry or something
    lua_newtable(L);
    return 1;
}
static int fr_gettenv(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTHREAD);
    lua_State* thread = lua_tothread(L, 1);

    lua_pushvalue(thread, LUA_GLOBALSINDEX);
    lua_xmove(thread, L, 1);
    return 1;
}

static int fr_printidentity(lua_State* L) {
    Task* task = getTask(L);
    assert(task);

    pushFunctionFromLookup(L, fr_print);
    lua_pushfstring(L, "Current identity is %d", task->identity->id);

    lua_call(L, 1, 0);

    return 0;
}
static int fr_getthreadidentity(lua_State* L) {
    Task* task = getTask(L);
    assert(task);

    lua_pushnumber(L, task->identity->id);
    return 1;
}
static int fr_setthreadidentity(lua_State* L) {
    Task* task = getTask(L);
    assert(task);

    int identity = luaL_checkinteger(L, 1);

    const auto& it = identity_map.find(identity);
    if (it == identity_map.end())
        luaL_error(L, "%d is not a valid identity", identity);

    task->identity = it->second;

    return 0;
}

static int fr_tick(lua_State* L) {
    auto now = std::chrono::system_clock::now();

    lua_pushnumber(L, std::chrono::duration_cast<std::chrono::duration<double>>(now.time_since_epoch()).count());
    return 1;
}

void open_frostbyte_environment(lua_State *L) {
    // methodlookup
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, METHODLOOKUP);
    // rbxscriptconnection_methodlookup
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, RBXSCRIPTCONNECTION_METHODLOOKUP);

    // string list
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, STRINGLOOKUP);

    env_expose(print)
    env_expose(warn)

    env_expose(getexecutorname)
    env_expose(identifyexecutor)

    env_expose(Version)

    env_expose(getreg)
    env_expose(getgc)
    env_expose(getallthreads)
    env_expose(getrendersteppedlist)

    env_expose(rawtostring)

    env_expose(wait)
    // TODO: actual spawn implementation (see the docs)
    pushFunctionFromLookup(L, fr_task_spawn, "spawn");
    lua_setglobal(L, "spawn");

    env_expose(loadstring)

    env_expose(gcstep)
    env_expose(gcfull)

    env_expose(getnamecallmethod)
    env_expose(setnamecallmethod)

    env_expose(islclosure)
    env_expose(iscclosure)

    env_expose(setsafeenv)
    env_expose(issafeenv)

    env_alias(setsafeenv, setuntouched)
    env_alias(issafeenv, isuntouched)

    env_expose(getrawmetatable)
    env_expose(setrawmetatable)

    env_expose(iswindowactive)
    env_alias(iswindowactive, isrbxactive)
    env_alias(iswindowactive, isgameactive)

    env_expose(setfpscap)
    env_expose(getfpscap)

    env_expose(setwindowtitle)

    env_expose(setclipboard)
    env_expose(getclipboard)
    env_alias(setclipboard, toclipboard)

    {
    lua_pushvalue(L, LUA_GLOBALSINDEX);

    setfunctionfield(L, DrawEntry__index, "getrenderproperty");
    setfunctionfield(L, DrawEntry__newindex, "setrenderproperty");
    setfunctionfield(L, fr_isrenderobject, "isrenderobject");
    env_alias(isrenderobject, isrenderobj)
    setfunctionfield(L, DrawEntry_clear, "cleardrawcache");

    setfunctionfield(L, fireRBXScriptSignal, "firesignal");

    lua_pop(L, 1);
    }

    env_expose(getgenv)
    env_expose(getrenv)
    env_expose(gettenv)

    env_expose(printidentity)
    env_expose(getthreadidentity)
    env_expose(setthreadidentity)

    env_expose(tick)

    {
    lua_getglobal(L, "table");
    setfunctionfield(L, fr_trawfreeze, "rawfreeze");
    setfunctionfield(L, fr_trawunfreeze, "rawunfreeze");
    setfunctionfield(L, fr_tsetfrozen, "setfrozen");

    lua_pop(L, 1);
    }

    open_debuglib(L);
}

}; // namespace frostbyte
