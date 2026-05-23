#include "engine/datatypes/font.hpp"

#include "engine/datatypes/enum.hpp"

#include "common.hpp"
#include "fontloader.hpp"
#include "userdata.hpp"

#include "lua.h"
#include "lualib.h"

namespace frostbyte {

int pushFont(lua_State* L, std::string family, EnumItem* weight, EnumItem* style) {
    EngineFont* engine_font = static_cast<EngineFont*>(lua_newuserdatatagged(L, sizeof(EngineFont), userdata::Font));
    new(engine_font) EngineFont;
    engine_font->family = family;
    engine_font->weight = weight;
    engine_font->style = style;

    std::string name = family;
    // if (weight->value != 400) { // Regular
        name.push_back('-');
        name.append(weight->name);
    // }

    // TODO: family should start with either rbxasset or rbxassetid. rbxasset should be something like rbxasset://fonts/families/%.json, which will be what Font.fromName does to name
    // in this case, we should read the json and get the entry of faces[weight] and do lookup from faces[weight]["assetId"]
    // which should be rbxasset://fonts/filename.[otf/ttf] or an rbxassetid://%d.
    // rbxassetid://%d should fetch from the web and put in temp folder.
    // if family doesn't start with rbxasset, just print the failed to load warning message and load default

    Font* font = FontLoader::engine_font_map.at("Arimo-Regular");

    auto entry = FontLoader::engine_font_map.find(name);
    if (entry == FontLoader::engine_font_map.end()) {
        char message[100];
        snprintf(message, 100, "Font family %s failed to load", name.c_str());
        consoleLog(L, Console::Message::WARNING, message);
    } else
        font = entry->second;

    engine_font->font = font;

    userdata::getClassMetatable(L, userdata::Font);
    lua_setmetatable(L, -2);

    return 1;
}
int pushFont(lua_State* L, EngineFont engine_font) {
    return pushFont(L, engine_font.family, engine_font.weight, engine_font.style);
}

static int Font_fromName(lua_State* L) {
    std::string family = luaL_checkstring(L, 1);
    EnumItem* weight = &Enum::enum_map.at("FontWeight").item_map.at("Regular");
    EnumItem* style = &Enum::enum_map.at("FontStyle").item_map.at("Normal");

    if (!lua_isnoneornil(L, 2))
        weight = lua_checkenumitem(L, 2, "FontWeight");
    if (!lua_isnoneornil(L, 3))
        style = lua_checkenumitem(L, 3, "FontStyle");

    return pushFont(L, family, weight, style);
}

EngineFont* lua_checkfont(lua_State* L, int narg) {
    void* ud = userdata::check(L, narg, userdata::Font);

    return static_cast<EngineFont*>(ud);
}

static int Font__tostring(lua_State* L) {
    EngineFont* engine_font = lua_checkfont(L, 1);
    lua_pushfstring(L, "Font { Family = %s, Weight = %s, Style = %s }", engine_font->family.c_str(), engine_font->weight->name.c_str(), engine_font->style->name.c_str());

    return 1;
}

static int Font__index(lua_State* L) {
    EngineFont* engine_font = lua_checkfont(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strequal(key, "Family"))
        lua_pushstring(L, engine_font->family.c_str());
    else if (strequal(key, "Weight"))
        pushEnumItem(L, engine_font->weight);
    else if (strequal(key, "Style"))
        pushEnumItem(L, engine_font->style);
    else if (strequal(key, "Bold"))
        // TODO: Font Bold
        lua_pushboolean(L, false);
    else
        goto INVALID;

    return 1;

    INVALID:
    luaL_error(L, "%s is not a valid member of Font", key);
}
static int Font__newindex(lua_State* L) {
    EngineFont* engine_font = lua_checkfont(L, 1);
    const char* key = luaL_checkstring(L, 2);

    if (strequal(key, "Family"))
        engine_font->family.assign(luaL_checkstring(L, 3));
    else if (strequal(key, "Weight"))
        engine_font->weight = lua_checkenumitem(L, 3, "FontWeight");
    else if (strequal(key, "Style"))
        engine_font->style = lua_checkenumitem(L, 3, "FontStyle");
    else if (strequal(key, "Bold"))
        // TODO: Font Bold. note that .Bold = true will set Bold to true and, for example, change weight from Regular to Bold
        luaL_error(L, "INTERNAL ERROR: FOND BOLD NEWINDEX");
    else
        goto INVALID;

    // FIXME: reset Font with same logic as pushFont (use a separate function )

    return 0;

    INVALID:
    luaL_error(L, "%s is not a valid member of Font", key);
}

void open_fontlib(lua_State* L) {
    // Font
    lua_newtable(L);

    setfunctionfield(L, Font_fromName, "fromName");

    lua_setglobal(L, "Font");

    // metatable
    userdata::newClassMetatable(L, userdata::Font);
    setfunctionfield(L, Font__tostring, "__tostring");
    setfunctionfield(L, Font__index, "__index");
    setfunctionfield(L, Font__newindex, "__newindex");

    lua_pop(L, 1);

    lua_setuserdatadtor(L, userdata::Font, [](lua_State* L, void* ud) {
        EngineFont* engine_font = static_cast<EngineFont*>(ud);
        engine_font->~EngineFont();
    });
}

}; // namespace frostbyte
