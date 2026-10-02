#include "engine/classes/layercollector.hpp"
#include "engine/classes/instance.hpp"

namespace frostbyte {

void rbxInstance_LayerCollector_init() {
    auto& this_class = rbxClass::class_map.at("LayerCollector");

    this_class->constructor = [](lua_State* L, std::shared_ptr<rbxInstance> instance) {
        setInstanceValue(instance, L, "Enabled", true, true);
    };
}

}; // namespace frostbyte
