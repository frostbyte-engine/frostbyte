#include "engine/classes/textshared.hpp"
#include "engine/classes/instance.hpp"
#include "engine/datatypes/enum.hpp"

namespace frostbyte {

bool newindexHookPre(lua_State* L, std::shared_ptr<rbxInstance> instance, const char* key) {
    if (strequal(key, "FontSize")) {
        auto value = lua_checkenumitem(L, 3, "FontSize");

        setInstanceValue(instance, L, "TextSize", static_cast<float>(std::atof(value->name.c_str() + 4)));

        return true;
    }

    return false;
}

void rbxInstance_TextShared_init() {
    auto& text_button = rbxClass::class_map.at("TextButton");
    auto& text_box = rbxClass::class_map.at("TextBox");
    auto& text_label = rbxClass::class_map.at("TextLabel");

    assert(!text_button->newindexHookPre);
    assert(!text_box->newindexHookPre);
    assert(!text_label->newindexHookPre);

    text_button->newindexHookPre = newindexHookPre;
    text_box->newindexHookPre = newindexHookPre;
    text_label->newindexHookPre = newindexHookPre;
}

}; // namespace frostbyte
