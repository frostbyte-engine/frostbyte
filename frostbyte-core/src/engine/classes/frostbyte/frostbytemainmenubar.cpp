#include "engine/classes/frostbyte/frostbytemainmenubar.hpp"

#include "common.hpp"
#include "engine/classes/instance.hpp"

namespace frostbyte {

bool main_menu_bar_enabled = true;

void FrostbyteMainMenuBar_init(lua_State *L) {
    auto this_class = std::make_shared<rbxClass>();
    this_class->name.assign("FrostbyteMainMenubar");
    this_class->tags |= rbxClass::NotCreatable;
    this_class->superclass = rbxClass::class_map.at("Instance");

    this_class->newProperty("Enabled", Primitive, { .value = main_menu_bar_enabled });
    this_class->setValueHookPost = [](lua_State* L, std::shared_ptr<rbxInstance> instance, const char* property, rbxValueVariant& value, bool is_from_lua) {
        if (!is_from_lua)
            return;
        if (strequal(property, "Enabled"))
            main_menu_bar_enabled = std::get<bool>(value);
    };

    rbxClass::class_map.try_emplace("FrostbyteMainMenuBar", this_class);
}

}; // namespace frostbyte

