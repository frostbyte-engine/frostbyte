#pragma once

#include "lua.h"

namespace frostbyte {

struct FrostbyteConfiguration {
    bool sandbox_enabled = true;
    const char* home_path = nullptr;
    void (*initializeWindow)() = nullptr;
    void (*cleanupWindow)() = nullptr;
    void (*imguiBegin)() = nullptr;
    void (*imguiEnd)() = nullptr;
};

class Frostbyte {
public:
    Frostbyte() = delete;

    static bool has_started;
    static lua_State* L;
    static lua_State* appL;

    static FrostbyteConfiguration configuration;
    static void initialize(FrostbyteConfiguration configuration);
    static void cleanup(bool restart = false);

    static bool isRunning();

    static void preRender();
    static void beginRender();
    static void endRender();
    static void postRender();
private:
    static bool running;
};


}; // namespace frostbyte
