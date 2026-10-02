#include "engine/datatypes/path2dcontrolpoint.hpp"

#include "common.hpp"
#include "engine/datatypes/udim2.hpp"
#include "userdata.hpp"

#include "lua.h"
#include "lualib.h"

namespace frostbyte {

int pushPath2DControlPoint(lua_State* L, UDim2 position, UDim2 left_tangent, UDim2 right_tangent) {
    Path2DControlPoint* path2dcontrolpoint = static_cast<Path2DControlPoint*>(lua_newuserdatatagged(L, sizeof(Path2DControlPoint), userdata::Path2DControlPoint));
    path2dcontrolpoint->position = position;
    path2dcontrolpoint->left_tangent = left_tangent;
    path2dcontrolpoint->right_tangent = right_tangent;

    userdata::getClassMetatable(L, userdata::Path2DControlPoint);
    lua_setmetatable(L, -2);

    return 1;
}
int pushPath2DControlPoint(lua_State* L, UDim2 position) {
    return pushPath2DControlPoint(L, position, UDim2{}, UDim2{});
}

static int Path2DControlPoint_new(lua_State* L) {
    const int narg = lua_gettop(L);
    if (narg == 0)
        return frostbyte::pushPath2DControlPoint(L, UDim2{});
    else if (narg == 1)
        return frostbyte::pushPath2DControlPoint(L, *lua_checkudim2(L, 1));
    else if (narg == 3)
        return frostbyte::pushPath2DControlPoint(L, *lua_checkudim2(L, 1), *lua_checkudim2(L, 3), *lua_checkudim2(L, 3));

    // TODO: error message
    luaL_error(L, "too many arguments to Path2DControlPoint.new (got %d, expected 3)", narg);
}

bool lua_ispath2dcontrolpoint(lua_State* L, int index) {
    return userdata::is(L, index, userdata::Path2DControlPoint);
}
Path2DControlPoint* lua_checkpath2dcontrolpoint(lua_State* L, int index) {
    void* ud = userdata::check(L, index, userdata::Path2DControlPoint);

    return static_cast<Path2DControlPoint*>(ud);
}

static int Path2DControlPoint__tostring(lua_State* L) {
    Path2DControlPoint* path2dcontrolpoint = lua_checkpath2dcontrolpoint(L, 1);

    // sorry
    lua_pushfstringL(L, "Path2DControlPoint { Position = {%.f, %.f}, {%.f, %.f}, LeftTangent = {%.f, %.f}, {%.f, %.f}, RightTangent = {%.f, %.f}, {%.f, %.f} }", path2dcontrolpoint->position.x.scale, path2dcontrolpoint->position.x.offset, path2dcontrolpoint->position.y.scale, path2dcontrolpoint->position.y.offset, path2dcontrolpoint->left_tangent.x.scale, path2dcontrolpoint->left_tangent.x.offset, path2dcontrolpoint->left_tangent.y.scale, path2dcontrolpoint->left_tangent.y.offset, path2dcontrolpoint->right_tangent.x.scale, path2dcontrolpoint->right_tangent.x.offset, path2dcontrolpoint->right_tangent.y.scale, path2dcontrolpoint->right_tangent.y.offset);
    return 1;
}

static int Path2DControlPoint__index(lua_State* L) {
    Path2DControlPoint* path2dcontrolpoint = lua_checkpath2dcontrolpoint(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strequal(key, "Position"))
        pushUDim2(L, path2dcontrolpoint->position);
    else if (strequal(key, "LeftTangent"))
        pushUDim2(L, path2dcontrolpoint->left_tangent);
    else if (strequal(key, "RightTangent"))
        pushUDim2(L, path2dcontrolpoint->right_tangent);
    else
        goto INVALID;

    return 1;

    INVALID:
    luaL_error(L, "%s is not a valid member of Path2DControlPoint", key);
}
static int Path2DControlPoint__newindex(lua_State* L) {
    Path2DControlPoint* path2dcontrolpoint = lua_checkpath2dcontrolpoint(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strequal(key, "Position"))
        path2dcontrolpoint->position = *lua_checkudim2(L, 3);
    else if (strequal(key, "LeftTangent"))
        path2dcontrolpoint->left_tangent = *lua_checkudim2(L, 3);
    else if (strequal(key, "RightTangent"))
        path2dcontrolpoint->right_tangent = *lua_checkudim2(L, 3);
    else
        goto INVALID;

    return 1;

    INVALID:
    luaL_error(L, "%s is not a valid member of Path2DControlPoint", key);

    return 0;
}

void open_path2dcontrolpointlib(lua_State *L) {
    // Path2DControlPoint
    lua_newtable(L);

    setfunctionfield(L, Path2DControlPoint_new, "new", true);

    lua_setglobal(L, "Path2DControlPoint");

    // metatable
    userdata::newClassMetatable(L, userdata::Path2DControlPoint);
    setfunctionfield(L, Path2DControlPoint__tostring, "__tostring");
    setfunctionfield(L, Path2DControlPoint__index, "__index");
    setfunctionfield(L, Path2DControlPoint__newindex, "__newindex");

    lua_pop(L, 1);
}

}; // namespace frostbyte

