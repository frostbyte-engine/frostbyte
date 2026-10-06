#include "engine/classes/frostbyte/frostbyteservice.hpp"

#include "engine/classes/serviceprovider.hpp"

namespace frostbyte {

#ifndef FROSTBYTE_HEADLESS
std::shared_ptr<rbxInstance> FrostbyteService::menu_bar = nullptr;
#endif
std::shared_ptr<rbxInstance> FrostbyteService::options = nullptr;

void FrostbyteService_init(lua_State* L, std::shared_ptr<rbxInstance> datamodel) {
    auto this_class = std::make_shared<rbxClass>();
    this_class->name.assign("FrostbyteService");
    this_class->tags |= rbxClass::NotCreatable;
    this_class->superclass = rbxClass::class_map.at("Instance");

    #ifndef FROSTBYTE_HEADLESS
    this_class->newProperty("MainMenuBar", Instance, { .value = std::shared_ptr<rbxInstance>() });
    #endif
    this_class->newProperty("Options", Instance, { .value = std::shared_ptr<rbxInstance>() });

    rbxClass::class_map.try_emplace("FrostbyteService", this_class);
    ServiceProvider::registerService("FrostbyteService");
    auto this_service = ServiceProvider::getService(L, datamodel, "FrostbyteService");

    #ifndef FROSTBYTE_HEADLESS
    FrostbyteService::menu_bar = newInstance(L, "FrostbyteMainMenuBar", this_service);
    setInstanceValue<std::shared_ptr<rbxInstance>>(this_service, L, "MainMenuBar",
                     FrostbyteService::menu_bar, true);
    #endif
    FrostbyteService::options = newInstance(L, "FrostbyteOptions", this_service);
    setInstanceValue<std::shared_ptr<rbxInstance>>(this_service, L, "Options",
                     FrostbyteService::options, true);

}

}

