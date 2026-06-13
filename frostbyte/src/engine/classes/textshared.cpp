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
    assert(!rbxClass::class_map["TextButton"]->newindexHookPre);
    assert(!rbxClass::class_map["TextBox"]->newindexHookPre);
    assert(!rbxClass::class_map["TextLabel"]->newindexHookPre);

    rbxClass::class_map["TextButton"]->newindexHookPre = newindexHookPre;
    rbxClass::class_map["TextBox"]->newindexHookPre = newindexHookPre;
    rbxClass::class_map["TextLabel"]->newindexHookPre = newindexHookPre;
}

}; // namespace frostbyte
