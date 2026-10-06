#include "engine/classes/frostbyte/frostbyteoptions.hpp"

#include "engine/classes/instance.hpp"

#include "common.hpp"
#include "frostbyteoption.hpp"
#include "taskscheduler.hpp"

#include "libraries/instructionlib.hpp"
#include "ui/ui.hpp"

namespace frostbyte {

std::vector<FrostbyteOptionUpdate> option_update_list;

struct Item {
    const char* property;
    FrostbyteOption& option;
};
Item list[] = {
    { "PrintToStdout", print_stdout },
    { "EnableStephook", enable_stephook },
    { "IsServer", runservice_is_server },
    { "IsStudio", runservice_is_studio },
};

// TODO: should PrintToStdout be readonly in headless builds?

void FrostbyteOptions_init(lua_State *L) {
    for (const auto& item : list)
        item.option.frostbyteservice_option_name = item.property;

    auto this_class = std::make_shared<rbxClass>();
    this_class->name.assign("FrostbyteOptions");
    this_class->tags |= rbxClass::NotCreatable;
    this_class->superclass = rbxClass::class_map.at("Instance");

    this_class->newProperty("Sandboxing", Primitive, { .value = TaskScheduler::sandboxing })
        ->tags |= rbxProperty::ReadOnly;

    for (const auto& item : list)
        this_class->newProperty(item.property, Primitive, { .value = static_cast<bool>(item.option) });

    this_class->setValueHookPost = [](lua_State* L, std::shared_ptr<rbxInstance> instance, const char* property, rbxValueVariant& value, bool is_from_lua) {
        if (!is_from_lua)
            return;

        for (const auto& item : list)
            if (strequal(item.property, property))
                item.option = std::get<bool>(value);
    };

    rbxClass::class_map.try_emplace("FrostbyteOptions", this_class);
}

}; // namespace frostbyte


