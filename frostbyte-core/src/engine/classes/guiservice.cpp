#include "engine/classes/guiservice.hpp"
#include "engine/classes/instance.hpp"
#include "engine/datatypes/vector2.hpp"

#include "ui/ui.hpp"

#include "lua.h"

namespace frostbyte {

namespace rbxInstance_GuiService_methods {
static int getGuiInset(lua_State* L) {
    pushVector2(L, gui_inset_topleft);
    pushVector2(L, gui_inset_bottomright);
    return 2;
}
};

void rbxInstance_GuiService_init() {
    auto& this_class = rbxClass::class_map.at("GuiService");

    this_class->methods.at("GetGuiInset").func = rbxInstance_GuiService_methods::getGuiInset;
}

}; // namespace frostbyte
