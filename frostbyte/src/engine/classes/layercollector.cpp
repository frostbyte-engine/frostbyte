#include "engine/classes/layercollector.hpp"
#include "engine/classes/instance.hpp"

namespace frostbyte {

void rbxInstance_LayerCollector_init() {
    rbxClass::class_map["LayerCollector"]->constructor = [](lua_State* L, std::shared_ptr<rbxInstance> instance) {
        setInstanceValue(instance, L, "Enabled", true, true);
    };
}

}; // namespace frostbyte
