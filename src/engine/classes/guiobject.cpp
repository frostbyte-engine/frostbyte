#include "engine/classes/guiobject.hpp"
#include "engine/classes/datamodel.hpp"
#include "engine/classes/guibutton.hpp"
#include "engine/classes/instance.hpp"
#include "engine/classes/serviceprovider.hpp"
#include "engine/classes/tweenservice.hpp"
#include "engine/datatypes/tweeninfo.hpp"
#include "engine/datatypes/udim2.hpp"

#include "lualib.h"

namespace frostbyte {

static int tweenProperties(lua_State* L, UDim2* position, UDim2* size) {
    const bool both = position && size;
    auto instance = lua_checkinstance(L, 1);

    TweenInfo tween_info;
    tween_info.easing_direction = &Enum::enum_map.at("EasingDirection").item_map.at("Out");
    tween_info.easing_style = &Enum::enum_map.at("EasingStyle").item_map.at("Quad");
    tween_info.time = 1;
    // bool override = false;

    if (!lua_isnoneornil(L, 3 + both))
        tween_info.easing_direction = lua_checkenumitem(L, 3 + both, "EasingDirection");
    if (!lua_isnoneornil(L, 4 + both))
        tween_info.easing_style = lua_checkenumitem(L, 4 + both, "EasingStyle");
    if (!lua_isnoneornil(L, 5 + both))
        tween_info.time = luaL_checknumberrange(L, 5 + both, 0, static_cast<uint32_t>(-1), "time");
    if (!lua_isnoneornil(L, 6 + both))
        /*override = */luaL_checkboolean(L, 6 + both);

    // TODO: access the c function directly
    auto game = DataModel::instance;
    auto tween_service = ServiceProvider::getService(L, game, "TweenService");
    lua_pushcfunction(L, tween_service->methods["Create"].func, "Create");

    lua_pushinstance(L, tween_service);
    lua_pushvalue(L, 1);
    pushTweenInfo(L, tween_info);

    lua_createtable(L, 0, 1);
    if (position) {
        pushUDim2(L, *position);
        lua_rawsetfield(L, -2, "Position");
    }
    if (size) {
        pushUDim2(L, *size);
        lua_rawsetfield(L, -2, "Size");
    }

    lua_call(L, 4, 1);

    auto tween_instance = lua_checkinstance(L, -1);

    /* NOTE: as per the docs, override is irrelevant... but i will keep the code here that would mimic the behavior
    bool would_interrupt = TweenService::wouldTweenInterrupt(L, tween_instance);

    if (!would_interrupt || override) {
        TweenService::activateTween(L, tween_instance);
        lua_pushboolean(L, true);
        return 1;
    }

    lua_pushboolean(L, false);
    return 1;
    */

    // TODO: callback

    TweenService::activateTween(L, tween_instance);
    lua_pushboolean(L, true);
    return 1;
}
namespace rbxInstance_GuiObject_methods {
    static int tweenPosition(lua_State* L) {
        return tweenProperties(L, lua_checkudim2(L, 2), nullptr);
    }
    static int tweenSize(lua_State* L) {
        return tweenProperties(L, nullptr, lua_checkudim2(L, 2));
    }
    static int tweenSizeAndPosition(lua_State* L) {
        return tweenProperties(L, lua_checkudim2(L, 2), lua_checkudim2(L, 3));
    }
}; // namespace rbxInstance_GuiObject_methods

void rbxInstance_GuiObject_init() {
    auto& this_class = rbxClass::class_map["GuiObject"];

    rbxClass::class_map["GuiObject"]->methods["TweenPosition"].func = rbxInstance_GuiObject_methods::tweenPosition;
    rbxClass::class_map["GuiObject"]->methods["TweenSize"].func = rbxInstance_GuiObject_methods::tweenSize;
    rbxClass::class_map["GuiObject"]->methods["TweenSizeAndPosition"].func = rbxInstance_GuiObject_methods::tweenSizeAndPosition;

    this_class->constructor = [](lua_State* L, std::shared_ptr<rbxInstance> instance) {
        setInstanceValue(instance, L, "BackgroundColor3", Color { 163, 162, 165, 255 }, true);
        setInstanceValue(instance, L, "BorderColor3", Color { 27, 42, 53, 255 }, true);
        setInstanceValue(instance, L, "BorderSizePixel", 1, true);
        setInstanceValue(instance, L, "Visible", true, true);
    };

    rbxInstance_GuiButton_init();
}

}; // namespace frostbyte
