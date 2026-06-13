#include "frostbyte.hpp"
#include <cfloat>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "engine/classes/baseplayergui.hpp"
#include "engine/classes/camera.hpp"
#include "engine/classes/datamodel.hpp"
#include "engine/classes/runservice.hpp"
#include "engine/classes/tweenservice.hpp"
#include "engine/classes/userinputservice.hpp"
#include "engine/classes/frostbyte/imguiservice.hpp"
#include "engine/classes/workspace.hpp"
#include "engine/datatypes/brickcolor.hpp"
#include "engine/datatypes/color3.hpp"
#include "engine/datatypes/colorsequence.hpp"
#include "engine/datatypes/colorsequencekeypoint.hpp"
#include "engine/datatypes/content.hpp"
#include "engine/datatypes/font.hpp"
#include "engine/datatypes/numberrange.hpp"
#include "engine/datatypes/numbersequence.hpp"
#include "engine/datatypes/numbersequencekeypoint.hpp"
#include "engine/datatypes/rbxscriptsignal.hpp"
#include "engine/datatypes/rect.hpp"
#include "engine/datatypes/tweeninfo.hpp"
#include "engine/datatypes/udim.hpp"
#include "engine/datatypes/udim2.hpp"
#include "engine/datatypes/vector2.hpp"
#include "engine/datatypes/vector3.hpp"

#include "libraries/cachelib.hpp"
#include "libraries/cryptlib.hpp"
#include "libraries/drawentrylib.hpp"
#include "libraries/drawingimmediate.hpp"
#include "libraries/filesystemlib.hpp"
#include "libraries/instructionlib.hpp"

#include "ui/ui.hpp"
#include "ui/functionexplorer.hpp"
#include "ui/imageexplorer.hpp"

#include "imguitheme.hpp"
#include "sysutils.hpp"

#include "curl/curl.h"
#include "raylib.h"
#include "imgui.h"

#include "common.hpp"
#include "basedrawing.hpp"
#include "taskscheduler.hpp"
#include "environment.hpp"
#include "console.hpp"
#include "fontloader.hpp"
#include "imageloader.hpp"

#include "lua.h"
#include "lualib.h"

namespace frostbyte {

std::string readFileToString(const char* file_path) {
    std::ifstream file(file_path);
    if (!file)
        throw std::runtime_error("failed to open file");

    std::string result;
    std::string buffer;
    while (std::getline(file, buffer))
        result.append(buffer) += '\n';
    if (result.size() > 0)
        result.erase(result.size() - 1);

    file.close();

    return result;
}

Shader round_shader;

bool Frostbyte::running = false;

bool Frostbyte::isRunning() {
    return running;
}

bool Frostbyte::has_started = false;
lua_State* Frostbyte::L = nullptr;
lua_State* Frostbyte::appL = nullptr;
FrostbyteConfiguration Frostbyte::configuration;

void Frostbyte::initialize(FrostbyteConfiguration configuration) {
    bool had_started = has_started;
    has_started = true;

    Frostbyte::configuration = configuration;

    TaskScheduler::init_time = lua_clock();
    TaskScheduler::sandboxing = configuration.sandbox_enabled;

    {
    if (!configuration.home_path)
        throw new std::runtime_error("please provide a valid home_path configuration");
    FileSystem::home_path = configuration.home_path;

    FileSystem::workspace_path.assign(FileSystem::home_path);
    FileSystem::workspace_path.append("workspace/");

    FileSystem::bin_path.assign(FileSystem::home_path);
    FileSystem::bin_path.append("bin/");

    FileSystem::temp_path.assign(FileSystem::home_path);
    FileSystem::temp_path.append("temp/");

    if (!std::filesystem::exists(FileSystem::home_path) && !std::filesystem::create_directory(FileSystem::home_path))
        throw new std::runtime_error("failed to create folder at $HOME/frostbyte");
    if (!std::filesystem::exists(FileSystem::workspace_path) && !std::filesystem::create_directory(FileSystem::workspace_path))
        throw new std::runtime_error("failed to create folder at $HOME/frostbyte/workspace");
    if (!std::filesystem::exists(FileSystem::bin_path) && !std::filesystem::create_directory(FileSystem::bin_path))
        throw new std::runtime_error("failed to create folder at $HOME/frostbyte/bin");
    if (!std::filesystem::exists(FileSystem::temp_path) && !std::filesystem::create_directory(FileSystem::temp_path))
        throw new std::runtime_error("failed to create folder at $HOME/frostbyte/temp");

    ScriptLanguage::refresh();

    // FIXME: we need to pull assets from the repo if the assets folder doesn't exist!

    }

    std::string api_dump;
    {
    std::string path = FileSystem::home_path;
    path.append("assets/Full-API-Dump.json");
    try {
        api_dump.assign(readFileToString(path.c_str()));
    } catch (std::exception& e) {
        throw new std::runtime_error(std::string("failed to read api dump at ").append(path).append(": ").append(e.what()));
    }
    }

    if (!had_started)
        curl_global_init(CURL_GLOBAL_DEFAULT);

    L = luaL_newstate();
    luaL_openlibs(L);

    TaskScheduler::setup(L);

    Console::ScriptConsole.debugf("global state: %p", L->global);
    Console::ScriptConsole.debugf("main state: %p", L);

    lua_createtable(L, 0, 1);
    lua_pushstring(L, "kvs");
    lua_setfield(L, -2, "__mode");
    lua_setfield(L, LUA_REGISTRYINDEX, "weakmetatable");

    open_frostbyte_environment(L);

    if (!configuration.initializeWindow)
        throw new std::runtime_error("please provide a valid initializeWindow callback");
    if (!configuration.cleanupWindow)
        throw new std::runtime_error("please provide a valid cleanupWindow callback");
    configuration.initializeWindow();

    FontLoader::load();

    open_filesystemlib(L);
    open_brickcolorlib(L);
    open_color3lib(L);
    open_colorsequencekeypointlib(L);
    open_colorsequencelib(L);
    open_contentlib(L);
    open_fontlib(L);
    open_tasklib(L);
    open_tweeninfolib(L);
    open_numberrangelib(L);
    open_numbersequencekeypointlib(L);
    open_numbersequencelib(L);
    open_rectlib(L);
    open_udimlib(L);
    open_udim2lib(L);
    open_vector2lib(L);
    open_vector3lib(L);

    initializeSharedPtrDestructorList(L);

    appL = TaskScheduler::newThread(L, [] (std::string error) { Console::ScriptConsole.error(error); });
    lua_pop(L, 1);
    Console::ScriptConsole.debugf("app state: %p", appL);
    rbxInstanceSetup(L, api_dump);
    rbxInstance::destructorL = appL;

    // the following need to happen after rbxInstance setup
    open_instructionlib(L);
    open_cachelib(L);
    open_cryptlib(L);
    UI_FunctionExplorer_init(L, DataModel::instance);
    ImGuiService_init(L, DataModel::instance);

    open_drawentrylib(L);
    open_drawingimmediate(L);

    lua_newtable(L);
    lua_setglobal(L, "shared");

    if (TaskScheduler::sandboxing)
        luaL_sandbox(L);

    lua_getglobal(L, "shared");
    lua_setreadonly(L, -1, false);

    lua_singlestep(L, true); // needed for stephook

    // load round shader
    {
        std::string base_path = FileSystem::home_path;
        base_path.append("assets/base.vs");
        std::string rounded_path = FileSystem::home_path;
        rounded_path.append("assets/rounded_rectangle.fs");

        round_shader = LoadShader(base_path.c_str(), rounded_path.c_str());

        if (!IsShaderValid(round_shader))
            throw new std::runtime_error(std::string("failed to load shaders at ").append(base_path).append(" and ").append(rounded_path));
    }

    // initialize round shader values
    {
        float shader_zero = 0.f;
        float vec4[] = { 5.f, 5.f, 5.f, 5.f };
        float vec2[] = { 0.f, 0.f };
        SetShaderValue(round_shader, GetShaderLocation(round_shader, "radius"), vec4, SHADER_UNIFORM_VEC4);
        SetShaderValue(round_shader, GetShaderLocation(round_shader, "shadowRadius"), &shader_zero, SHADER_UNIFORM_FLOAT);
        SetShaderValue(round_shader, GetShaderLocation(round_shader, "shadowOffset"), vec2, SHADER_UNIFORM_VEC2);
        SetShaderValue(round_shader, GetShaderLocation(round_shader, "shadowScale"), &shader_zero, SHADER_UNIFORM_FLOAT);
        SetShaderValue(round_shader, GetShaderLocation(round_shader, "borderThickness"), &shader_zero, SHADER_UNIFORM_FLOAT);
    }

    default_imgui_style = ImGui::GetStyle();
    changeImGuiTheme(imgui_theme);

    DataModel::onLoad(L);

    running = true;
}
void Frostbyte::cleanup(bool restart) {
    DataModel::onShutdown(appL);
    running = false;

    UnloadShader(round_shader);

    for (auto& entry : DrawEntry::draw_list)
        entry->free();

    FontLoader::unload();
    UI_ImageExplorer_cleanup();
    ImageLoader::unload();
    // NOTE: we already check for the existance of cleanupWindow when checking for initializeWindow
    configuration.cleanupWindow();

    rbxInstanceCleanup(appL);
    rbxScriptSignal::cleanup();

    // for (int i = 0; i < IM_ARRAYSIZE(protected_thread_list); i++)
    //     TaskScheduler::killThread(protected_thread_list[i]);

    // probably redundant?
    TaskScheduler::cleanup();

    lua_close(L);

    if (!restart)
        curl_global_cleanup();
}

void Frostbyte::preRender() {
    UserInputService::any_imgui = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
    if (enable_user_input_service)
        UserInputService::process(appL);
    if (enable_run_service)
        RunService::process(appL);
    if (enable_tween_service)
        TweenService::process(appL);

    int screen_width = GetScreenWidth();
    int screen_height = GetScreenHeight();
    rbxCamera::screen_size.x = screen_width;
    rbxCamera::screen_size.y = screen_height;

    // camera
    rbxInstance_Camera_updateViewport(appL);
}
void Frostbyte::beginRender() {
    BeginDrawing();
    ClearBackground(DARKGRAY);

    // gui object render
    if (enable_gui_object_rendering)
        rbxInstance_BasePlayerGui_render(appL);

    // lua drawings
    DrawEntry::render();
    render_drawingimmediate(appL);
}
void Frostbyte::endRender() {
    if (show_fps)
        DrawFPS(30, 30);

    EndDrawing();
}
void Frostbyte::postRender() {
    TaskScheduler::run();

    {
    std::lock_guard lock(TaskScheduler::pending_yield_mutex);

    while (!TaskScheduler::pending_yield_list.empty()) {
        auto& yield = TaskScheduler::pending_yield_list.front();
        auto thread = yield.state;

        int arg_count = 0;
        try {
            arg_count = yield.finisher(thread);
        } catch (std::exception& e) {
            getTask(thread)->feedback(e.what());
        }

        TaskScheduler::pending_yield_list.pop();

        if (arg_count == YIELD_ERROR) {
            getTask(thread)->feedback(getErrorMessage(thread));
            TaskScheduler::killThread(thread);
        // TODO: if finisher throws exception, do we kill the thread?
        } else if (arg_count == YIELD_KILL)
            TaskScheduler::killThread(thread);
        else
            TaskScheduler::queueForResume(thread, arg_count);
    }
    }

    RunService::heartbeat(appL);

    setInstanceValue<double>(Workspace::instance, appL, "DistributedGameTime", lua_clock() - TaskScheduler::init_time);

    SysUtils::run();
}

}; // namespace frostbyte
