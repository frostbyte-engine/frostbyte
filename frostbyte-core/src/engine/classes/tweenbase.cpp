#include "engine/classes/tweenbase.hpp"
#include "engine/classes/tweenservice.hpp"
#include <cstdlib>

namespace frostbyte {

namespace rbxInstance_TweenBase_methods {
    static int cancel(lua_State* L) {
        auto instance = lua_checkinstance(L, 1, "TweenBase");

        TweenService::cancelTween(L, instance);
        return 0;
    }
    static int play(lua_State* L) {
        auto instance = lua_checkinstance(L, 1, "TweenBase");

        TweenService::activateTween(L, instance);
        return 0;
    }
    static int pause(lua_State* L) {
        auto instance = lua_checkinstance(L, 1, "TweenBase");

        TweenService::pauseTween(L, instance);
        return 0;
    }
}; // rbxInstance_TweenBase_methods

void rbxInstance_TweenBase_init() {
    auto& this_class = rbxClass::class_map.at("TweenBase");

    this_class->constructor = [](lua_State* L, std::shared_ptr<rbxInstance> instance) {
        setInstanceValue(instance, L, "PlaybackState", &Enum::enum_map.at("PlaybackState").item_map.at("Begin"));
    };
    this_class->destructor = [](rbxInstance* instance) {
        TweenObject* tween_object = (TweenObject*)instance->getValue<void*>("internal_Object");
        if (tween_object)
            tween_object->~TweenObject();
    };

    this_class->methods.at("Cancel").func = rbxInstance_TweenBase_methods::cancel;
    this_class->methods.at("Play").func = rbxInstance_TweenBase_methods::play;
    this_class->methods.at("Pause").func = rbxInstance_TweenBase_methods::pause;

    this_class->newInternalProperty("internal_Object", Primitive, { .value = (void*) nullptr });
}

}; // namespace frostbyte
