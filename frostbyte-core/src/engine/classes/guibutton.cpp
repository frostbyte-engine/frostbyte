#include "engine/classes/guibutton.hpp"

namespace frostbyte {

std::map<rbxInstance*, bool> auto_button_color_map;

void rbxInstance_GuiButton_init() {
    auto& this_class = rbxClass::class_map.at("GuiButton");

    this_class->constructor = [](lua_State* L, std::shared_ptr<rbxInstance> instance) {
        setInstanceValue(instance, L, "AutoButtonColor", true, true);
    };

    this_class->newProperty("internal_CanActivate", Primitive, { .value = false }, true);
    this_class->newProperty("internal_ActivateCount", Primitive, { .value = int32_t(0) }, true);
    this_class->newProperty("internal_Click1Step1", Primitive, { .value = false }, true);
    this_class->newProperty("internal_Click2Step1", Primitive, { .value = false }, true);
}

bool checkAutoButtonColor(std::shared_ptr<rbxInstance> instance) {
    if (!instance->isA("GuiButton"))
        return false;
    if (!getInstanceValue<bool>(instance, "AutoButtonColor"))
        return false;

    // FIXME: when we make image buttons, we need to return false if we're doing a mouse hover and hoverimage is valid OR if we're doing a click and pressedimage is valid
    // (so we'll also need to intake a parameter differentiating hover vs click)

    // if (instance->isA("ImageButton") && isImageButtonImageValid(instance))
    //     return false;

    return true;
}
void handleGuiButtonMouseEnter(std::shared_ptr<rbxInstance> instance) {
    if (!checkAutoButtonColor(instance))
        return;

    auto_button_color_map[instance.get()] = true;
}
void handleGuiButtonMouseLeave(std::shared_ptr<rbxInstance> instance) {
    if (!checkAutoButtonColor(instance))
        return;

    auto_button_color_map[instance.get()] = false;
}

}; // namespace frostbyte
