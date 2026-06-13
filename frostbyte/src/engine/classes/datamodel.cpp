#include "engine/classes/datamodel.hpp"
#include "engine/datatypes/rbxscriptsignal.hpp"

#include "common.hpp"
#include "taskscheduler.hpp"
#include "http.hpp"

#include "lualib.h"
#include "ui/ui.hpp"

namespace frostbyte {

std::shared_ptr<rbxInstance> DataModel::instance;
bool DataModel::loaded = false;
bool DataModel::shutdown = false;

#define TOCLOSEBINDS_KEY "datamodeltoclosebinds"

void DataModel::onLoad(lua_State* L) {
    assert(instance);

    loaded = true;

    pushFunctionFromLookup(L, fireRBXScriptSignal);
    instance->pushSignal(L, "Loaded", true);

    lua_call(L, 1, 0);
}
void DataModel::onShutdown(lua_State* L) {
    assert(instance);

    pushFunctionFromLookup(L, fr_task_spawn, "spawn");

    auto& on_close = getInstanceValue<rbxCallback>(instance, "OnClose");

    if (on_close.index != -1) {
        lua_pushvalue(L, -1);
        lua_getfield(L, LUA_REGISTRYINDEX, METHODLOOKUP);
        lua_rawgeti(L, -1, on_close.index);
        lua_remove(L, -2);

        lua_call(L, 1, 0);
    }

    lua_getfield(L, LUA_REGISTRYINDEX, TOCLOSEBINDS_KEY);
    lua_pushnil(L);
    while (lua_next(L, -2)) {
        lua_pushvalue(L, -4); // task.spawn
        lua_pushvalue(L, -2); // value

        lua_call(L, 1, 0);

        lua_pop(L, 1);
    }

    // NOTE: Close is part of ServiceProvider not DataModel
    pushFunctionFromLookup(L, fireRBXScriptSignal);
    instance->pushSignal(L, "Close", true);

    lua_call(L, 1, 0);
}

void httpGetInternal(const char* url, Yield yield) {
    MemoryStruct chunk;

    newGetRequest(url, &chunk);

    yield.finish([chunk] (lua_State* L) {
        if (chunk.res)
            luaL_errorL(L, "failed to make HTTP GET request (%d)", chunk.res);

        lua_pushlstring(L, chunk.memory, chunk.size);
        if (chunk.memory) free(chunk.memory);

        return 1;
    });
}

namespace rbxInstance_DataModel_methods {
    static int shutdown(lua_State* L) {
        lua_checkinstance(L, 1, "DataModel");

        DataModel::shutdown = true;

        printf("DataModel:Shutdown() called!\n");

        return 0;
    }
    static int bindToClose(lua_State* L) {
        lua_checkinstance(L, 1, "DataModel");
        luaL_checktype(L, 2, LUA_TFUNCTION);

        lua_getfield(L, LUA_REGISTRYINDEX, TOCLOSEBINDS_KEY);

        lua_pushvalue(L, 2);
        lua_rawseti(L, 3, lua_objlen(L, 3) + 1);

        return 0;
    }
    static int isLoaded(lua_State* L) {
        lua_checkinstance(L, 1, "DataModel");

        lua_pushboolean(L, DataModel::loaded);
        return 1;
    }

    static int httpGet(lua_State* L) {
        lua_checkinstance(L, 1, "DataModel");
        std::string url = luaL_checkstring(L, 2);

        bool synchronous = httpget_synchronous_argument && luaL_optboolean(L, 3, false);

        return TaskScheduler::yieldForWork(L, [url] (Yield yield) {
            httpGetInternal(url.c_str(), yield);
        }, !synchronous);
    }
    static int httpGetAsync(lua_State* L) {
        lua_checkinstance(L, 1, "DataModel");
        std::string url = luaL_checkstring(L, 2);

        return TaskScheduler::yieldForWork(L, [url] (Yield yield) {
            httpGetInternal(url.c_str(), yield);
        }, true);
    }
}; // namespace rbxInstance_DataModel_methods

void rbxInstance_DataModel_init(lua_State* L) {
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, TOCLOSEBINDS_KEY);

    rbxClass::class_map["DataModel"]->methods["BindToClose"].func = rbxInstance_DataModel_methods::bindToClose;
    rbxClass::class_map["DataModel"]->methods["HttpGetAsync"].func = rbxInstance_DataModel_methods::httpGetAsync;
    rbxClass::class_map["DataModel"]->methods["IsLoaded"].func = rbxInstance_DataModel_methods::isLoaded;
    rbxClass::class_map["DataModel"]->methods["Shutdown"].func = rbxInstance_DataModel_methods::shutdown;

    rbxClass::class_map["DataModel"]->newMethod("HttpGet", rbxInstance_DataModel_methods::httpGet);
}

};
