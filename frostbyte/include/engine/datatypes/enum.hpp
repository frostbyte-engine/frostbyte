#pragma once

#include <map>
#include <string>

#include "lua.h"

namespace frostbyte {

class EnumItem {
public:
    std::string name;
    std::string enum_name;
    unsigned int value = 0;
    int lookup = 0;
};

class Enum {
public:
    static std::map<std::string, Enum> enum_map;

    std::string name;
    std::map<std::string, EnumItem> item_map;
    int lookup = 0;
};

int pushEnumItem(lua_State* L, EnumItem* enum_item);

EnumItem* getEnumItemFromValue(const char* enum_name, unsigned int value);

bool lua_isenumitem(lua_State* L, int narg);
EnumItem* lua_checkenumitem(lua_State* L, int narg, const char* expected_enum);

void setup_enums(lua_State* L);

}; // namespace frostbyte
