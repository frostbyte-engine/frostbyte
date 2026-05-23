#pragma once

#include <cstdint>
#include <functional>
#include <shared_mutex>
#include <string>
#include <cstring> // for strcmp for strequal

#include "nlohmann/json.hpp"
#include "console.hpp"
#include "userdata.hpp"

#include "lobject.h"
#include "lua.h"
#include "lstate.h"

using json = nlohmann::json;

namespace frostbyte {

// NOTE: this is hard-coded sorry
#define MAX_KEYBOARD_KEYS 512

#define strequal(str1, str2) (strcmp(str1, str2) == 0)

#define luaL_optnumberloose(L, narg, def) (luaL_opt(L, lua_tonumber, narg, def))

// from laux.cpp
const char* currfuncname(lua_State* L);

extern bool print_stdout;

int countDecimals(double value);
#define decimalFmt(value) countDecimals(value), value

double getSeconds(lua_State* L, int arg = 1);

void initializeSharedPtrDestructorList(lua_State* L);

using Feedback = std::function<void(std::string)>;
using OnKill = std::function<void()>;

std::string fixString(std::string_view original);

std::string rawtostringobj(lua_State* L, const TValue* obj, bool use_fixstring = false);
std::string rawtostring(lua_State* L, int index);

double luaL_checknumberrange(lua_State* L, int narg, double min, double max, const char* context);
double luaL_optnumberrange(lua_State* L, int narg, double min, double max, const char* context, double def = 0);

int createweaktable(lua_State* L, int narr, int nrec);
int newweaktable(lua_State* L);

int pushFromLookup(lua_State* L, const char* lookup, std::function<void()> pushKey, std::function<void()> pushValue);
int pushFromLookup(lua_State* L, const char* lookup, void* ptr, std::function<void()> pushValue);

int pushFunctionFromLookup(lua_State* L, lua_CFunction func, const char* name = nullptr, lua_Continuation cont = nullptr);
// push lookup, call addToLookup, lookup is popped by addToLookup
int addToLookup(lua_State *L, std::function<void()> pushValue, bool keep_value = false);

template<class T>
void pushNewSharedPtrObject(lua_State* L, std::shared_ptr<T>& ptr, int ttag) {
    void* object = lua_newuserdatatagged(L, sizeof(std::shared_ptr<T>), ttag);
    new(object) std::shared_ptr<T>(ptr);
}

template<class T>
int pushFromSharedPtrLookup(lua_State* L, const char* lookup, std::shared_ptr<T>& ptr, int ttag) {
    return pushFromLookup(L, lookup, ptr.get(), [&L, &ptr, &ttag](){
        pushNewSharedPtrObject(L, ptr, ttag);

        userdata::getClassMetatable(L, ttag);
        lua_setmetatable(L, -2);
    });
}

#define INSTANCELOOKUP "instancelookup"
#define METHODLOOKUP "methodlookup"
#define RBXSCRIPTCONNECTION_METHODLOOKUP "rbxscriptconnectionmethodlookup"
// TODO: I don't think we use the string lookup for any non-static strings, so it should be removed which will become easily when we get rid of redundant std::string uses
#define STRINGLOOKUP "stringlookup"
#define SIGNALLOOKUP "signallookup"
#define SIGNALCONNECTIONLISTLOOKUP "signalconnectionlistlookup"
#define ENUMLOOKUP "enumlookup"

// TODO: replace as many std::string functon parameters as are feasible
int addToStringLookup(lua_State* L, std::string string);
const char* getFromStringLookup(lua_State* L, int index);

std::string getStackMessage(lua_State* L);

void consoleLog(lua_State* L, Console::Message::Type type, std::string message);
int fr_print(lua_State* L);
int fr_warn(lua_State* L);

// if you keep lookup as false, NOTE that FunctionExplorer will ensure functions are in the lookup when explored
void setfunctionfield(lua_State* L, lua_CFunction func, const char* funcname, const char* debugname, bool lookup = false);
// if you keep lookup as false, NOTE that FunctionExplorer will ensure functions are in the lookup when explored
void setfunctionfield(lua_State* L, lua_CFunction func, const char* funcname, bool lookup = false);

std::string sha1ToString(unsigned int* hashed);

}; // namespace frostbyte
