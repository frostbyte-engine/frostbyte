#include "engine/classes/bindableevent.hpp"
#include "engine/classes/instance.hpp"
#include "engine/datatypes/rbxscriptsignal.hpp"

#include <memory>

namespace frostbyte {

namespace rbxInstance_BindableEvent_methods {
    static int fire(lua_State* L) {
        std::shared_ptr<rbxInstance> instance = lua_checkinstance(L, 1, "BindableEvent");
        lua_getfield(L, 1, "Event");
        lua_remove(L, 1);
        lua_insert(L, 1);

        fireRBXScriptSignal(L);

        return 0;
    }
};

void rbxInstance_BindableEvent_init() {
    rbxClass::class_map["BindableEvent"]->methods.at("Fire").func = rbxInstance_BindableEvent_methods::fire;
}

}; // namespace frostbyte
