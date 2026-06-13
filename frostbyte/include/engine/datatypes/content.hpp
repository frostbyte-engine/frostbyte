#pragma once

#include "engine/classes/instance.hpp"

#include "lua.h"

#include <string>
#include <variant>

namespace frostbyte {

using ContentValue = std::variant<std::monostate, std::string, std::shared_ptr<rbxInstance>>;

struct Content {
    enum Type {
        Uri,
        Object,
        Opaque
    } type;
    ContentValue value;
};

int pushContent(lua_State* L, ContentValue value);
Content* lua_checkcontent(lua_State* L, int narg);

void open_contentlib(lua_State* L);

}; // namespace frostbyte
