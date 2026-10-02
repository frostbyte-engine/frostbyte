#include "engine/classes/httpservice.hpp"
#include "common.hpp"
#include "engine/classes/instance.hpp"

#include <random>

#include "lapi.h"
#include "lgc.h"
#include "lobject.h"
#include "ltable.h"
#include "lualib.h"

namespace frostbyte {

std::mt19937_64 rng_device;
std::string generateUUID() {
    std::array<uint8_t, 16> bytes;
    std::uniform_int_distribution<uint32_t> dist(0, 255);
    for (auto& b : bytes)
        b = static_cast<uint8_t>(dist(rng_device));

    bytes[6] = (bytes[6] & 0x0F) | 0x40;
    bytes[8] = (bytes[8] & 0x3F) | 0x80;

    std::ostringstream oss;
    oss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; ++i) {
        oss << std::setw(2) << static_cast<int>(bytes[i]);
        if (i == 3 || i == 5 || i == 7 || i == 9)
            oss << '-';
    }
    return oss.str();
}

int pushJsonToLua(lua_State* L, json& json_object) {
    switch (json_object.type()) {
        case json::value_t::null:
        case json::value_t::discarded:
        case json::value_t::binary:
            lua_pushnil(L);
            break;
        case json::value_t::object:
            lua_createtable(L, 0, json_object.size());

            for (const auto& item: json_object.items()) {
                pushJsonToLua(L, item.value());
                lua_rawsetfield(L, -2, item.key().c_str());
            }

            break;
        case json::value_t::array:
            lua_createtable(L, json_object.size(), 0);

            for (size_t i = 0; i < json_object.size(); i++) {
                pushJsonToLua(L, json_object[i]);
                lua_rawseti(L, -2, i + 1);
            }

            break;
        case json::value_t::string: {
            std::string string = json_object.get<std::string>();
            lua_pushlstring(L, string.c_str(), string.size());
            break;
        } case json::value_t::boolean:
            lua_pushboolean(L, json_object.get<bool>());
            break;
        case json::value_t::number_integer:
            lua_pushinteger(L, json_object.get<int>());
            break;
        case json::value_t::number_unsigned:
            lua_pushinteger(L, json_object.get<unsigned int>());
            break;
        case json::value_t::number_float:
            lua_pushnumber(L, json_object.get<float>());
            break;
    }

    return 1;
}

void luaToJson(luaL_Strbuf* buf, int index) {
    lua_State* L = buf->L;
    switch (lua_type(L, index)) {
        case LUA_TBOOLEAN:
        case LUA_TNUMBER:
            luaL_addvalueany(buf, index);
            break;
        case LUA_TSTRING: {
            std::string fixed = fixString(lua_tostring(L, index));
            luaL_addlstring(buf, fixed.c_str(), fixed.size());
            break;
        } case LUA_TTABLE: {
            bool is_dictionary = false;

            // NOTE: I am using hvalue and manual traversal because I got an error with lua_next that I COULD NOT fix

            LuaTable* h = hvalue(luaA_toobject(L, index));
            int sizearray = h->sizearray;
            int sizenode = 1 << h->lsizenode;

            for (int i = 0; i < sizenode; i++) {
                LuaNode* n = &h->node[i];

                if (!ttisnil(gval(n))) {
                    is_dictionary = true;
                    break;
                }
            }

            luaL_addchar(buf, is_dictionary ? '{' : '[');

            bool empty = true;
            if (is_dictionary) {
                for (int i = 0; i < sizenode; i++) {
                    LuaNode* n = &h->node[i];

                    if (!ttisnil(gval(n))) {
                        lua_pushnil(L);
                        getnodekey(L, L->top - 1, n);

                        if (lua_type(L, -1) != LUA_TSTRING) {
                            lua_pop(L, 1);
                            goto CONTINUE;
                        }

                        empty = false;

                        luaToJson(buf, -1);
                        luaL_addstring(buf, ": ");

                        // the below pop and pushnil are probably redundant but I am being cautious right now cuz this crap isn't working
                        lua_pop(L, 1);

                        lua_pushnil(L);
                        setobj2s(L, L->top - 1, gval(n));

                        luaToJson(buf, -1);
                        luaL_addstring(buf, ", ");

                        lua_pop(L, 1);
                    }

                    CONTINUE:
                    index++;
                }
            } else {
                empty = !sizearray;

                for (int i = 0; i < sizearray; i++) {
                    TValue* e = &h->array[i];

                    if (ttisnil(e)) {
                        luaL_addstring(buf, "null, ");
                    } else {
                        lua_pushnil(L);
                        setobj2s(L, L->top - 1, e);

                        luaToJson(buf, -1);
                        luaL_addstring(buf, ", ");

                        lua_pop(L, 1);
                    }
                }
            }

            // TODO: idk if removing characters like this is fine (like idk what the expectations of a buf object are in Luau), we should probably just use our own normal char buffer and push it to the stack
            // remove trailing ", "
            if (!empty)
                buf->p -= 2;

            luaL_addchar(buf, is_dictionary ? '}' : ']');
            break;
        }

        default:
            luaL_addstring(buf, "null");
            break;
    }
}

namespace rbxInstance_HttpService_methods {
    static int generateGUID(lua_State* L) {
        lua_checkinstance(L, 1, "HttpService");
        const bool wrap = luaL_optboolean(L, 2, true);

        std::string uuid = generateUUID();

        if (wrap) {
            uuid.insert(uuid.begin(), '{');
            uuid.push_back('}');
        }

        lua_pushstring(L, uuid.c_str());
        return 1;
    }

    static int jsonDecode(lua_State* L) {
        lua_checkinstance(L, 1, "HttpService");

        size_t l;
        const char* rawjson = luaL_checklstring(L, 2, &l);

        json root_json = json::parse(rawjson);
        return pushJsonToLua(L, root_json);
    }
    static int jsonEncode(lua_State* L) {
        lua_checkinstance(L, 1, "HttpService");

        luaL_checktype(L, 2, LUA_TTABLE);

        luaL_Strbuf buf;
        luaL_buffinit(L, &buf);

        luaToJson(&buf, 2);

        luaL_pushresult(&buf);
        return 1;
    }
}; // namespace rbxInstance_HttpService_methods

void rbxInstance_HttpService_init() {
    auto& this_class = rbxClass::class_map.at("HttpService");

    this_class->methods.at("GenerateGUID").func = rbxInstance_HttpService_methods::generateGUID;
    this_class->methods.at("JSONDecode").func = rbxInstance_HttpService_methods::jsonDecode;
    this_class->methods.at("JSONEncode").func = rbxInstance_HttpService_methods::jsonEncode;
}

}; // namespace frostbyte
