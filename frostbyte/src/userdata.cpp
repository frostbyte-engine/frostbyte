#include "userdata.hpp"
#include "common.hpp"

#include "lapi.h"
#include "lobject.h"
#include "lstate.h"
#include "lualib.h"

namespace frostbyte {
    namespace userdata {
        #define X(name) #name,
        const char* const userdata_names[] = {
            "SharedPtr",
            USERDATA_TYPES
        };
        #undef X

        bool get(lua_State* L, void*& out, int ud, int ttag) {
            if (lua_isnone(L, ud))
                return false;

            const TValue* o = luaA_toobject(L, ud);
            if (ttype(o) != LUA_TUSERDATA || uvalue(o)->tag != ttag)
                return false;

            out = uvalue(o)->data;
            return true;
        }

        bool is(lua_State* L, int ud, int ttag) {
            void* result;
            return get(L, result, ud, ttag);
        }

        void* check(lua_State* L, int ud, int ttag) {
            void* result;
            if (!get(L, result, ud, ttag))
                luaL_typeerrorL(L, ud, userdata_names[ttag]);

            return result;
        }

        int newClassMetatable(lua_State* L, int ttag) {
            luaL_newmetatable(L, userdata_names[ttag]);
            lua_pushstring(L, userdata_names[ttag]);
            lua_rawsetfield(L, -2, "__type");

            return 1;
        }
        int getClassMetatable(lua_State* L, int ttag) {
            luaL_getmetatable(L, userdata_names[ttag]);

            return 1;
        }
    }
}
