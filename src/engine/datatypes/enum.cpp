#include "engine/datatypes/enum.hpp"

#include "common.hpp"
#include "userdata.hpp"

#include "lua.h"
#include "lualib.h"

namespace frostbyte {

std::map<std::string, Enum> Enum::enum_map;

int pushEnumTable(lua_State* L, std::string name) {
    lua_rawgetfield(L, LUA_REGISTRYINDEX, ENUMLOOKUP);

    Enum* _enum = &Enum::enum_map.at(name);
    int* lookup = &_enum->lookup;

    if (*lookup) {
        lua_rawgeti(L, -1, *lookup);
        lua_remove(L, -2);
        return 1;
    }

    *lookup = addToLookup(L, [&L, &_enum] {
        lua_createtable(L, 2, 0);

        Enum** enum_ptr = static_cast<Enum**>(lua_newuserdatatagged(L, sizeof(Enum*), userdata::Enum));
        *enum_ptr = _enum;

        userdata::getClassMetatable(L, userdata::Enum);
        lua_setmetatable(L, -2);

        lua_rawseti(L, -2, 1);

        lua_newtable(L);
        lua_rawseti(L, -2, 2);
    }, true);
    return 1;
}
int pushEnum(lua_State* L, std::string name) {
    assert(pushEnumTable(L, name) == 1);
    assert(lua_rawgeti(L, -1, 1) != LUA_TNIL);
    return 1;
}

int pushEnumItem(lua_State* L, EnumItem* enum_item) {
    assert(pushEnumTable(L, enum_item->enum_name.c_str()) == 1);
    assert(lua_rawgeti(L, -1, 2) != LUA_TNIL);
    lua_remove(L, -2);

    int* lookup = &enum_item->lookup;

    if (*lookup) {
        lua_rawgeti(L, -1, *lookup);
        lua_remove(L, -2);
        return 1;
    }

    *lookup = addToLookup(L, [&L, &enum_item] {
        EnumItem** enum_item_ptr = static_cast<EnumItem**>(lua_newuserdatatagged(L, sizeof(EnumItem*), userdata::EnumItem));
        *enum_item_ptr = enum_item;

        userdata::getClassMetatable(L, userdata::EnumItem);
        lua_setmetatable(L, -2);
    }, true);
    return 1;
}
int pushEnumItem(lua_State* L, std::string enum_name, std::string name) {
    return pushEnumItem(L, &Enum::enum_map.at(enum_name).item_map.at(name));
}

EnumItem* getEnumItemFromValue(const char* enum_name, unsigned int value) {
    auto& e = Enum::enum_map.at(enum_name);
    for (auto it = e.item_map.begin(); it != e.item_map.end(); it++)
        if (it->second.value == value)
            return &it->second;
    return nullptr;
}

Enum* lua_checkenum(lua_State* L, int narg) {
    void* ud = userdata::check(L, narg, userdata::Enum);
    return *static_cast<Enum**>(ud);
}

static int Enum__tostring(lua_State* L) {
    Enum* _enum = lua_checkenum(L, 1);

    lua_pushfstring(L, "%s", _enum->name.c_str());
    return 1;
}

namespace Enum_methods {
    static int getEnumItems(lua_State* L) {
        if (lua_gettop(L) > 1)
            luaL_error(L, "too many arguments to GetEnumItems! expected 1");

        Enum* _enum = lua_checkenum(L, 1);

        lua_createtable(L, _enum->item_map.size(), 0);

        int i = 1;
        for (auto const& item : _enum->item_map) {
            pushEnumItem(L, _enum->name, item.first);
            lua_rawseti(L, 2, i++);
        }

        return 1;
    }
    static int fromName(lua_State* L) {
        Enum* _enum = lua_checkenum(L, 1);
        const char* key = luaL_checkstring(L, 2);

        if (_enum->item_map.find(key) == _enum->item_map.end()) {
            lua_pushnil(L);
            return 1;
        }

        return pushEnumItem(L, _enum->name, key);
    }
    static int fromValue(lua_State* L) {
        Enum* _enum = lua_checkenum(L, 1);
        luaL_checktype(L, 2, LUA_TNUMBER);
        int value = lua_tonumber(L, 2);

        if (EnumItem* enum_item = getEnumItemFromValue(_enum->name.c_str(), value))
            return pushEnumItem(L, _enum->name, enum_item->name);

        lua_pushnil(L);
        return 1;
    }
}
lua_CFunction getEnumMethod(Enum* entry, const char* key) {
    if (strequal(key, "GetEnumItems"))
        return Enum_methods::getEnumItems;
    else if (strequal(key, "FromName"))
        return Enum_methods::fromName;
    else if (strequal(key, "FromValue"))
        return Enum_methods::fromValue;

    return nullptr;
}

static int Enum__index(lua_State* L) {
    Enum* _enum = lua_checkenum(L, 1);
    const char* key = luaL_checkstring(L, 2);

    lua_CFunction func = getEnumMethod(_enum, key);
    if (func)
        return pushFunctionFromLookup(L, func, key);

    if (_enum->item_map.find(key) == _enum->item_map.end())
        luaL_error(L, "%s is not a valid member of \"Enum.%s\"", key, _enum->name.c_str());

    return pushEnumItem(L, _enum->name, key);
}

static int Enum__namecall(lua_State* L) {
    Enum* _enum = lua_checkenum(L, 1);
    const char* namecall = lua_namecallatom(L, nullptr);
    if (!namecall)
        luaL_error(L, "no namecall method!");

    lua_CFunction func = getEnumMethod(_enum, namecall);
    if (!func)
        luaL_error(L, "%s is not a valid member of Enum", namecall);

    return func(L);
}

EnumItem* checkEnumItem(lua_State* L, int narg) {
    void* ud = userdata::check(L, narg, userdata::EnumItem);
    return *static_cast<EnumItem**>(ud);
}
EnumItem* lua_checkenumitem(lua_State* L, int narg, const char* expected_enum) {
    // we need an expected_enum because we also have value and name checks
    assert(expected_enum);
    auto& e = Enum::enum_map.at(expected_enum);

    {
        int is_num;
        unsigned i = lua_tointegerx(L, narg, &is_num);
        if (is_num) {
            if (EnumItem* enum_item = getEnumItemFromValue(expected_enum, i))
                return enum_item;
            luaL_error(L, "Invalid value %d for enum %s", i, expected_enum);
        }
    }

    {
        size_t strl;
        const char* str = lua_tolstring(L, narg, &strl);
        if (str) {
            auto result = e.item_map.find(str);
            if (result == e.item_map.end())
                luaL_error(L, "Invalid value \"%.*s\" for enum %s", static_cast<int>(strl), str, expected_enum);
            return &result->second;
        }
    }

    return checkEnumItem(L, narg);
}

static int EnumItem__tostring(lua_State* L) {
    EnumItem* enum_item = checkEnumItem(L, 1);

    lua_pushfstring(L, "Enum.%s.%s", enum_item->enum_name.c_str(), enum_item->name.c_str());
    return 1;
}

static int EnumItem__index(lua_State* L) {
    EnumItem* enum_item = checkEnumItem(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strequal(key, "Name"))
        lua_pushstring(L, enum_item->name.c_str());
    else if (strequal(key, "Value"))
        lua_pushunsigned(L, enum_item->value);
    else if (strequal(key, "EnumType"))
        pushEnum(L, enum_item->enum_name);
    else
        luaL_errorL(L, "%s is not a valid member of Enum.%s.%s", key, enum_item->enum_name.c_str(), enum_item->name.c_str());

    return 1;
}
static int EnumItem__eq(lua_State* L) {
    EnumItem* a = checkEnumItem(L, 1);
    EnumItem* b = checkEnumItem(L, 2);

    lua_pushboolean(L, a->enum_name == b->enum_name && a->value == b->value);
    return 1;
}

void lua_checkenums(lua_State* L, int narg) {
    userdata::check(L, narg, userdata::Enums);
}

static int Enums__tostring(lua_State* L) {
    lua_checkenums(L, 1);

    lua_pushstring(L, "Enums");
    return 1;
}

static int Enums__index(lua_State* L) {
    lua_checkenums(L, 1);
    const char* key = lua_tostring(L, 2);

    if (Enum::enum_map.find(key) == Enum::enum_map.end())
        luaL_error(L, "%s is not a valid member of \"Enum\"", key);

    return pushEnum(L, key);
}

void setup_enums(lua_State* L) {
    // enumlookup
    lua_createtable(L, 5, 0);
    lua_setfield(L, LUA_REGISTRYINDEX, ENUMLOOKUP);

    // Enum
    lua_newuserdatatagged(L, 0, userdata::Enums);

    // Enums metatable
    userdata::newClassMetatable(L, userdata::Enums);
    setfunctionfield(L, Enums__tostring, "__tostring", nullptr);
    setfunctionfield(L, Enums__index, "__index", nullptr);
    // TODO: Enums methods
    // setfunctionfield(L, Enums__namecall, "__namecall", nullptr);

    lua_setmetatable(L, -2);

    lua_setglobal(L, "Enum");

    // Enum metatable
    userdata::newClassMetatable(L, userdata::Enum);
    setfunctionfield(L, Enum__tostring, "__tostring", nullptr);
    setfunctionfield(L, Enum__index, "__index", nullptr);
    setfunctionfield(L, Enum__namecall, "__namecall", nullptr);

    lua_pop(L, 1);

    // EnumItem metatable
    userdata::newClassMetatable(L, userdata::EnumItem);
    setfunctionfield(L, EnumItem__tostring, "__tostring", nullptr);
    setfunctionfield(L, EnumItem__index, "__index", nullptr);
    setfunctionfield(L, EnumItem__eq, "__eq", nullptr);

    lua_pop(L, 1);
}

}; // namespace frostbyte
