#include "engine/datatypes/brickcolor.hpp"

#include "common.hpp"

#include "engine/datatypes/color3.hpp"
#include "lualib.h"

namespace frostbyte {

std::vector<BrickColor> brick_color_list = {
    {"White", 1, Color{242, 243, 243, 255}},
    {"Grey", 2, Color{161, 165, 162, 255}},
    {"Light yellow", 3, Color{249, 233, 153, 255}},
    {"Brick yellow", 5, Color{215, 197, 154, 255}},
    {"Light green (Mint)", 6, Color{194, 218, 184, 255}},
    {"Light reddish violet", 9, Color{232, 186, 200, 255}},
    {"Pastel Blue", 11, Color{128, 187, 219, 255}},
    {"Light orange brown", 12, Color{203, 132, 66, 255}},
    {"Nougat", 18, Color{204, 142, 105, 255}},
    {"Bright red", 21, Color{196, 40, 28, 255}},
    {"Med. reddish violet", 22, Color{196, 112, 160, 255}},
    {"Bright blue", 23, Color{13, 105, 172, 255}},
    {"Bright yellow", 24, Color{245, 205, 48, 255}},
    {"Earth orange", 25, Color{98, 71, 50, 255}},
    {"Black", 26, Color{27, 42, 53, 255}},
    {"Dark grey", 27, Color{109, 110, 108, 255}},
    {"Dark green", 28, Color{40, 127, 71, 255}},
    {"Medium green", 29, Color{161, 196, 140, 255}},
    {"Lig. Yellowich orange", 36, Color{243, 207, 155, 255}},
    {"Bright green", 37, Color{75, 151, 75, 255}},
    {"Dark orange", 38, Color{160, 95, 53, 255}},
    {"Light bluish violet", 39, Color{193, 202, 222, 255}},
    {"Transparent", 40, Color{236, 236, 236, 255}},
    {"Tr. Red", 41, Color{205, 84, 75, 255}},
    {"Tr. Lg blue", 42, Color{193, 223, 240, 255}},
    {"Tr. Blue", 43, Color{123, 182, 232, 255}},
    {"Tr. Yellow", 44, Color{247, 241, 141, 255}},
    {"Light blue", 45, Color{180, 210, 228, 255}},
    {"Tr. Flu. Reddish orange", 47, Color{217, 133, 108, 255}},
    {"Tr. Green", 48, Color{132, 182, 141, 255}},
    {"Tr. Flu. Green", 49, Color{248, 241, 132, 255}},
    {"Phosph. White", 50, Color{236, 232, 222, 255}},
    {"Light red", 100, Color{238, 196, 182, 255}},
    {"Medium red", 101, Color{218, 134, 122, 255}},
    {"Medium blue", 102, Color{110, 153, 202, 255}},
    {"Light grey", 103, Color{199, 193, 183, 255}},
    {"Bright violet", 104, Color{107, 50, 124, 255}},
    {"Br. yellowish orange", 105, Color{226, 155, 64, 255}},
    {"Bright orange", 106, Color{218, 133, 65, 255}},
    {"Bright bluish green", 107, Color{0, 143, 156, 255}},
    {"Earth yellow", 108, Color{104, 92, 67, 255}},
    {"Bright bluish violet", 110, Color{67, 84, 147, 255}},
    {"Tr. Brown", 111, Color{191, 183, 177, 255}},
    {"Medium bluish violet", 112, Color{104, 116, 172, 255}},
    {"Tr. Medi. reddish violet", 113, Color{229, 173, 200, 255}},
    {"Med. yellowish green", 115, Color{199, 210, 60, 255}},
    {"Med. bluish green", 116, Color{85, 165, 175, 255}},
    {"Light bluish green", 118, Color{183, 215, 213, 255}},
    {"Br. yellowish green", 119, Color{164, 189, 71, 255}},
    {"Lig. yellowish green", 120, Color{217, 228, 167, 255}},
    {"Med. yellowish orange", 121, Color{231, 172, 88, 255}},
    {"Br. reddish orange", 123, Color{211, 111, 76, 255}},
    {"Bright reddish violet", 124, Color{146, 57, 120, 255}},
    {"Light orange", 125, Color{234, 184, 146, 255}},
    {"Tr. Bright bluish violet", 126, Color{165, 165, 203, 255}},
    {"Gold", 127, Color{220, 188, 129, 255}},
    {"Dark nougat", 128, Color{174, 122, 89, 255}},
    {"Silver", 131, Color{156, 163, 168, 255}},
    {"Neon orange", 133, Color{213, 115, 61, 255}},
    {"Neon green", 134, Color{216, 221, 86, 255}},
    {"Sand blue", 135, Color{116, 134, 157, 255}},
    {"Sand violet", 136, Color{135, 124, 144, 255}},
    {"Medium orange", 137, Color{224, 152, 100, 255}},
    {"Sand yellow", 138, Color{149, 138, 115, 255}},
    {"Earth blue", 140, Color{32, 58, 86, 255}},
    {"Earth green", 141, Color{39, 70, 45, 255}},
    {"Tr. Flu. Blue", 143, Color{207, 226, 247, 255}},
    {"Sand blue metallic", 145, Color{121, 136, 161, 255}},
    {"Sand violet metallic", 146, Color{149, 142, 163, 255}},
    {"Sand yellow metallic", 147, Color{147, 135, 103, 255}},
    {"Dark grey metallic", 148, Color{87, 88, 87, 255}},
    {"Black metallic", 149, Color{22, 29, 50, 255}},
    {"Light grey metallic", 150, Color{171, 173, 172, 255}},
    {"Sand green", 151, Color{120, 144, 130, 255}},
    {"Sand red", 153, Color{149, 121, 119, 255}},
    {"Dark red", 154, Color{123, 46, 47, 255}},
    {"Tr. Flu. Yellow", 157, Color{255, 246, 123, 255}},
    {"Tr. Flu. Red", 158, Color{225, 164, 194, 255}},
    {"Gun metallic", 168, Color{117, 108, 98, 255}},
    {"Red flip/flop", 176, Color{151, 105, 91, 255}},
    {"Yellow flip/flop", 178, Color{180, 132, 85, 255}},
    {"Silver flip/flop", 179, Color{137, 135, 136, 255}},
    {"Curry", 180, Color{215, 169, 75, 255}},
    {"Fire Yellow", 190, Color{249, 214, 46, 255}},
    {"Flame yellowish orange", 191, Color{232, 171, 45, 255}},
    {"Reddish brown", 192, Color{105, 64, 40, 255}},
    {"Flame reddish orange", 193, Color{207, 96, 36, 255}},
    {"Medium stone grey", 194, Color{163, 162, 165, 255}},
    {"Royal blue", 195, Color{70, 103, 164, 255}},
    {"Dark Royal blue", 196, Color{35, 71, 139, 255}},
    {"Bright reddish lilac", 198, Color{142, 66, 133, 255}},
    {"Dark stone grey", 199, Color{99, 95, 98, 255}},
    {"Lemon metalic", 200, Color{130, 138, 93, 255}},
    {"Light stone grey", 208, Color{229, 228, 223, 255}},
    {"Dark Curry", 209, Color{176, 142, 68, 255}},
    {"Faded green", 210, Color{112, 149, 120, 255}},
    {"Turquoise", 211, Color{121, 181, 181, 255}},
    {"Light Royal blue", 212, Color{159, 195, 233, 255}},
    {"Medium Royal blue", 213, Color{108, 129, 183, 255}},
    {"Rust", 216, Color{144, 76, 42, 255}},
    {"Brown", 217, Color{124, 92, 70, 255}},
    {"Reddish lilac", 218, Color{150, 112, 159, 255}},
    {"Lilac", 219, Color{107, 98, 155, 255}},
    {"Light lilac", 220, Color{167, 169, 206, 255}},
    {"Bright purple", 221, Color{205, 98, 152, 255}},
    {"Light purple", 222, Color{228, 173, 200, 255}},
    {"Light pink", 223, Color{220, 144, 149, 255}},
    {"Light brick yellow", 224, Color{240, 213, 160, 255}},
    {"Warm yellowish orange", 225, Color{235, 184, 127, 255}},
    {"Cool yellow", 226, Color{253, 234, 141, 255}},
    {"Dove blue", 232, Color{125, 187, 221, 255}},
    {"Medium lilac", 268, Color{52, 43, 117, 255}},
    {"Slime green", 301, Color{80, 109, 84, 255}},
    {"Smoky grey", 302, Color{91, 93, 105, 255}},
    {"Dark blue", 303, Color{0, 16, 176, 255}},
    {"Parsley green", 304, Color{44, 101, 29, 255}},
    {"Steel blue", 305, Color{82, 124, 174, 255}},
    {"Storm blue", 306, Color{51, 88, 130, 255}},
    {"Lapis", 307, Color{16, 42, 220, 255}},
    {"Dark indigo", 308, Color{61, 21, 133, 255}},
    {"Sea green", 309, Color{52, 142, 64, 255}},
    {"Shamrock", 310, Color{91, 154, 76, 255}},
    {"Fossil", 311, Color{159, 161, 172, 255}},
    {"Mulberry", 312, Color{89, 34, 89, 255}},
    {"Forest green", 313, Color{31, 128, 29, 255}},
    {"Cadet blue", 314, Color{159, 173, 192, 255}},
    {"Electric blue", 315, Color{9, 137, 207, 255}},
    {"Eggplant", 316, Color{123, 0, 123, 255}},
    {"Moss", 317, Color{124, 156, 107, 255}},
    {"Artichoke", 318, Color{138, 171, 133, 255}},
    {"Sage green", 319, Color{185, 196, 177, 255}},
    {"Ghost grey", 320, Color{202, 203, 209, 255}},
    {"Lilac", 321, Color{167, 94, 155, 255}},
    {"Plum", 322, Color{123, 47, 123, 255}},
    {"Olivine", 323, Color{148, 190, 129, 255}},
    {"Laurel green", 324, Color{168, 189, 153, 255}},
    {"Quill grey", 325, Color{223, 223, 222, 255}},
    {"Crimson", 327, Color{151, 0, 0, 255}},
    {"Mint", 328, Color{177, 229, 166, 255}},
    {"Baby blue", 329, Color{152, 194, 219, 255}},
    {"Carnation pink", 330, Color{255, 152, 220, 255}},
    {"Persimmon", 331, Color{255, 89, 89, 255}},
    {"Maroon", 332, Color{117, 0, 0, 255}},
    {"Gold", 333, Color{239, 184, 56, 255}},
    {"Daisy orange", 334, Color{248, 217, 109, 255}},
    {"Pearl", 335, Color{231, 231, 236, 255}},
    {"Fog", 336, Color{199, 212, 228, 255}},
    {"Salmon", 337, Color{255, 148, 148, 255}},
    {"Terra Cotta", 338, Color{190, 104, 98, 255}},
    {"Cocoa", 339, Color{86, 36, 36, 255}},
    {"Wheat", 340, Color{241, 231, 199, 255}},
    {"Buttermilk", 341, Color{254, 243, 187, 255}},
    {"Mauve", 342, Color{224, 178, 208, 255}},
    {"Sunrise", 343, Color{212, 144, 189, 255}},
    {"Tawny", 344, Color{150, 85, 85, 255}},
    {"Rust", 345, Color{143, 76, 42, 255}},
    {"Cashmere", 346, Color{211, 190, 150, 255}},
    {"Khaki", 347, Color{226, 220, 188, 255}},
    {"Lily white", 348, Color{237, 234, 234, 255}},
    {"Seashell", 349, Color{233, 218, 218, 255}},
    {"Burgundy", 350, Color{136, 62, 62, 255}},
    {"Cork", 351, Color{188, 155, 93, 255}},
    {"Burlap", 352, Color{199, 172, 120, 255}},
    {"Beige", 353, Color{202, 191, 163, 255}},
    {"Oyster", 354, Color{187, 179, 178, 255}},
    {"Pine Cone", 355, Color{108, 88, 75, 255}},
    {"Fawn brown", 356, Color{160, 132, 79, 255}},
    {"Hurricane grey", 357, Color{149, 137, 136, 255}},
    {"Cloudy grey", 358, Color{171, 168, 158, 255}},
    {"Linen", 359, Color{175, 148, 131, 255}},
    {"Copper", 360, Color{150, 103, 102, 255}},
    {"Medium brown", 361, Color{86, 66, 54, 255}},
    {"Bronze", 362, Color{126, 104, 63, 255}},
    {"Flint", 363, Color{105, 102, 92, 255}},
    {"Dark taupe", 364, Color{90, 76, 66, 255}},
    {"Burnt Sienna", 365, Color{106, 57, 9, 255}},
    {"Institutional white", 1001, Color{248, 248, 248, 255}},
    {"Mid gray", 1002, Color{205, 205, 205, 255}},
    {"Really black", 1003, Color{17, 17, 17, 255}},
    {"Really red", 1004, Color{255, 0, 0, 255}},
    {"Deep orange", 1005, Color{255, 176, 0, 255}},
    {"Alder", 1006, Color{180, 128, 255, 255}},
    {"Dusty Rose", 1007, Color{163, 75, 75, 255}},
    {"Olive", 1008, Color{193, 190, 66, 255}},
    {"New Yeller", 1009, Color{255, 255, 0, 255}},
    {"Really blue", 1010, Color{0, 0, 255, 255}},
    {"Navy blue", 1011, Color{0, 32, 96, 255}},
    {"Deep blue", 1012, Color{33, 84, 185, 255}},
    {"Cyan", 1013, Color{4, 175, 236, 255}},
    {"CGA brown", 1014, Color{170, 85, 0, 255}},
    {"Magenta", 1015, Color{170, 0, 170, 255}},
    {"Pink", 1016, Color{255, 102, 204, 255}},
    {"Deep orange", 1017, Color{255, 175, 0, 255}},
    {"Teal", 1018, Color{18, 238, 212, 255}},
    {"Toothpaste", 1019, Color{0, 255, 255, 255}},
    {"Lime green", 1020, Color{0, 255, 0, 255}},
    {"Camo", 1021, Color{58, 125, 21, 255}},
    {"Grime", 1022, Color{127, 142, 100, 255}},
    {"Lavender", 1023, Color{140, 91, 159, 255}},
    {"Pastel light blue", 1024, Color{175, 221, 255, 255}},
    {"Pastel orange", 1025, Color{255, 201, 201, 255}},
    {"Pastel violet", 1026, Color{177, 167, 255, 255}},
    {"Pastel blue-green", 1027, Color{159, 243, 233, 255}},
    {"Pastel green", 1028, Color{204, 255, 204, 255}},
    {"Pastel yellow", 1029, Color{255, 255, 204, 255}},
    {"Pastel brown", 1030, Color{255, 204, 153, 255}},
    {"Royal purple", 1031, Color{98, 37, 209, 255}},
    {"Hot pink", 1032, Color{255, 0, 191, 255}}
};

BrickColor* getBrickColor(const char* name) {
    for (size_t i = 0; i < brick_color_list.size(); i++) {
        if (strequal(brick_color_list[i].name, name))
            return &brick_color_list[i];
    }

    return nullptr;
}
BrickColor* getBrickColor(int index) {
    for (size_t i = 0; i < brick_color_list.size(); i++) {
        if (brick_color_list[i].index == index)
            return &brick_color_list[i];
    }

    return nullptr;
}
BrickColor* getBrickColor(Color color) {
    for (size_t i = 0; i < brick_color_list.size(); i++) {
        if (brick_color_list[i].color.r == color.r && brick_color_list[i].color.g == color.g &&
            brick_color_list[i].color.b == color.b && brick_color_list[i].color.a == color.a
        )
            return &brick_color_list[i];
    }

    return nullptr;
}

int pushBrickColor(lua_State* L, const char* name) {
    BrickColor** brick_color = static_cast<BrickColor**>(lua_newuserdatatagged(L, sizeof(BrickColor*), userdata::BrickColor));
    *brick_color = getBrickColor(name);

    userdata::getClassMetatable(L, userdata::BrickColor);
    lua_setmetatable(L, -2);

    return 1;
}
int pushBrickColor(lua_State* L, int index) {
    BrickColor** brick_color = static_cast<BrickColor**>(lua_newuserdatatagged(L, sizeof(BrickColor*), userdata::BrickColor));
    *brick_color = getBrickColor(index);

    userdata::getClassMetatable(L, userdata::BrickColor);
    lua_setmetatable(L, -2);

    return 1;
}
int pushBrickColor(lua_State* L, Color color) {
    BrickColor** brick_color = static_cast<BrickColor**>(lua_newuserdatatagged(L, sizeof(BrickColor*), userdata::BrickColor));
    *brick_color = getBrickColor(color);

    userdata::getClassMetatable(L, userdata::BrickColor);
    lua_setmetatable(L, -2);

    return 1;
}

static int BrickColor_new(lua_State* L) {
    switch (lua_type(L, 1)) {
        case LUA_TNUMBER:
            if (lua_gettop(L) > 1) {
                Color color{
                    static_cast<unsigned char>(luaL_checknumber(L, 1) * 255.f),
                    static_cast<unsigned char>(luaL_checknumber(L, 2) * 255.f),
                    static_cast<unsigned char>(luaL_checknumber(L, 3) * 255.f),
                    255
                };
                return pushBrickColor(L, color);
            } else
                return pushBrickColor(L, lua_tonumber(L, 1));
        case LUA_TSTRING:
            return pushBrickColor(L, lua_tostring(L, 1));

        case LUA_TUSERDATA:
            return pushBrickColor(L, *lua_checkcolor(L, 1));
    }

    luaL_argerror(L, 1, "expected number, string, or Color3");
    return 0;
}
static int BrickColor_palette(lua_State* L) {
    return pushBrickColor(L, luaL_checknumber(L, 1));
}
static int BrickColor_random(lua_State* L) {
    // TODO: BrickColor.random
    return pushBrickColor(L, 211);
}

#define COLOR_FUNCTIONS             \
    X(White, "White")               \
    X(Gray, "Medium stone gray")    \
    X(DarkGray, "Dark stone grey")  \
    X(Black, "Black")               \
    X(Red, "Bright red")            \
    X(Yellow, "Bright yellow")      \
    X(Green, "Dark green")          \
    X(Blue, "Bright blue")

#define X(funcname, name) \
    static int BrickColor_##funcname(lua_State* L) { \
        return pushBrickColor(L, name); \
    }
COLOR_FUNCTIONS
#undef X

bool lua_isbrickcolor(lua_State* L, int narg) {
    return userdata::is(L, narg, userdata::BrickColor);
}
BrickColor* lua_checkbrickcolor(lua_State* L, int narg) {
    void* ud = userdata::check(L, narg, userdata::BrickColor);

    return *static_cast<BrickColor**>(ud);
}

void open_brickcolorlib(lua_State* L) {
    // BrickColor
    lua_newtable(L);

    setfunctionfield(L, BrickColor_new, "new", true);
    setfunctionfield(L, BrickColor_palette, "palette", true);
    setfunctionfield(L, BrickColor_random, "random", true);

    #define X(funcname, name) setfunctionfield(L, BrickColor_##funcname, #funcname, true);
    COLOR_FUNCTIONS
    #undef X

    lua_setglobal(L, "BrickColor");

    // metatable
    userdata::newClassMetatable(L, userdata::BrickColor);

    lua_pop(L, 1);
}

}; // namespace frostbyte
