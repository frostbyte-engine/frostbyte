#include "lua.h"
#include "lobject.h"
#include "lstate.h"

namespace frostbyte {

Closure* levelToClosure(lua_State* L, int level, CallInfo** ci_out = nullptr);

void open_debuglib(lua_State* L);

}
