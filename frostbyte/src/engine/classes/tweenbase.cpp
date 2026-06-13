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
    rbxClass::class_map["TweenBase"]->constructor = [](lua_State* L, std::shared_ptr<rbxInstance> instance) {
        setInstanceValue(instance, L, "PlaybackState", &Enum::enum_map.at("PlaybackState").item_map.at("Begin"));
    };
    rbxClass::class_map["TweenBase"]->destructor = [](rbxInstance* instance) {
        TweenObject* tween_object = (TweenObject*)instance->getValue<void*>("internal_Object");
        if (tween_object)
            tween_object->~TweenObject();
    };

    rbxClass::class_map["TweenBase"]->methods["Cancel"].func = rbxInstance_TweenBase_methods::cancel;
    rbxClass::class_map["TweenBase"]->methods["Play"].func = rbxInstance_TweenBase_methods::play;
    rbxClass::class_map["TweenBase"]->methods["Pause"].func = rbxInstance_TweenBase_methods::pause;

    rbxClass::class_map["TweenBase"]->newInternalProperty("internal_Object", Primitive, { .value = (void*) nullptr });
}

}; // namespace frostbyte
