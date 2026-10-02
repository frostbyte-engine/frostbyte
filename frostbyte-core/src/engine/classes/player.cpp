#include "engine/classes/player.hpp"

namespace frostbyte {

std::shared_ptr<rbxInstance> rbxPlayer::localplayer;
std::shared_ptr<rbxInstance> rbxPlayer::localmouse;

namespace rbxInstance_Player_methods {
    static int getMouse(lua_State* L) {
        lua_checkinstance(L, 1, "Player");
        return lua_pushinstance(L, rbxPlayer::localmouse);
    }
}; // namespace rbxInstance_Player_methods

void rbxInstance_Player_init(lua_State *L, std::shared_ptr<rbxInstance> players_service) {
    auto& this_class = rbxClass::class_map.at("Player");

    this_class->methods.at("GetMouse").func = rbxInstance_Player_methods::getMouse;

    rbxPlayer::localplayer = newInstance(L, "Player", players_service);
    rbxPlayer::localplayer->values.at(PROP_INSTANCE_NAME).value = "LocalPlayer";

    players_service->values.at("LocalPlayer").value = rbxPlayer::localplayer;

    rbxPlayer::localmouse = newInstance(L, "Mouse");
}

}; // namespace frostbyte
