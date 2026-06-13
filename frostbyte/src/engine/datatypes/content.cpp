#include "engine/datatypes/content.hpp"

#include "userdata.hpp"

#include "lualib.h"
#include <variant>

namespace frostbyte {

int pushContent(lua_State* L, ContentValue value) {
    Content* content = static_cast<Content*>(lua_newuserdatatagged(L, sizeof(Content), userdata::Content));
    content->value = value;

    if (std::holds_alternative<std::string>(value))
        content->type = Content::Uri;
    else if (std::holds_alternative<std::shared_ptr<rbxInstance>>(value))
        content->type = Content::Object;
    else if (std::holds_alternative<std::monostate>(value))
        content->type = Content::Opaque;

    userdata::getClassMetatable(L, userdata::Content);
    lua_setmetatable(L, -2);

    return 1;
}

static int Content_fromUri(lua_State* L) {
    const char* uri = luaL_checkstring(L, 1);
    return pushContent(L, std::string(uri));
}

Content* lua_checkcontent(lua_State* L, int narg) {
    return static_cast<Content*>(userdata::check(L, narg, userdata::Content));
}

static int Content__tostring(lua_State* L) {
    Content* content = lua_checkcontent(L, 1);

    luaL_Strbuf buf;
    luaL_buffinit(L, &buf);

    luaL_addstring(&buf, "Content{SourceType=");
    switch (content->type) {
        case Content::Uri:
            luaL_addstring(&buf, "Uri, Uri=");
            luaL_addstring(&buf, std::get<std::string>(content->value).data());
            break;
        case Content::Object:
            luaL_addstring(&buf, "Object");
            break;
        case Content::Opaque:
            luaL_addstring(&buf, "Opaque");
            break;
    }

    luaL_addchar(&buf, '}');

    luaL_pushresult(&buf);

    return 1;
}

static int Content__index(lua_State* L) {
    Content* content = lua_checkcontent(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strequal(key, "Uri")) {
        if (content->type == Content::Uri)
            lua_pushstring(L, std::get<std::string>(content->value).data());
        else
            lua_pushnil(L);
    } else if (strequal(key, "Object")) {
        // TODO: we uhhh don't have Objects in frostbyte cuz I'm lazy so..

        // if (content->type == Content::Object)
        //     lua_pushstring(L, std::get<std::string>(content->value).data());
        // else
        //     lua_pushnil(L);
        lua_pushnil(L);
    } else if (strequal(key, "Opaque")) {
        // if (content->type == Content::Opaque)
        //     lua_pushstring(L, std::get<std::string>(content->value).data());
        // else
        //     lua_pushnil(L);
        lua_pushnil(L);
    } else
        goto INVALID;

    return 1;

    INVALID:
    luaL_error(L, "%s is not a valid member of Content", key);
}
static int Content__newindex(lua_State* L) {
    lua_checkcontent(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strequal(key, "Uri") || strequal(key, "Object") ||
        strequal(key, "Opaque")
    )
        luaL_error(L, "%s member of Content is read-only and cannot be assigned to", key);

    luaL_error(L, "%s is not a valid member of Content", key);

    return 0;
}

void open_contentlib(lua_State* L) {
    // Content
    lua_newtable(L);

    setfunctionfield(L, Content_fromUri, "fromUri", true);

    lua_setglobal(L, "Content");

    // metatable
    userdata::newClassMetatable(L, userdata::Content);
    setfunctionfield(L, Content__tostring, "__tostring");
    setfunctionfield(L, Content__index, "__index");
    setfunctionfield(L, Content__newindex, "__newindex");

    lua_pop(L, 1);
}

}; // namespace frostbyte
