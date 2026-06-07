#include <cfloat>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <shared_mutex>
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
#include "ui/drawentrylist.hpp"
#include "ui/instanceexplorer.hpp"
#include "ui/functionexplorer.hpp"
#include "ui/imageexplorer.hpp"
#include "ui/fontexplorer.hpp"
#include "ui/tableexplorer.hpp"

#include "imguitheme.hpp"
#include "sysutils.hpp"

#include "curl/curl.h"
#include "raylib.h"
#include "rlImGui.h"
#include "imgui.h"
#include "ImGuiFileDialog.h"

#include "common.hpp"
#include "basedrawing.hpp"
#include "taskscheduler.hpp"
#include "cli.hpp"
#include "environment.hpp"
#include "console.hpp"
#include "tests.hpp"
#include "fontloader.hpp"
#include "imageloader.hpp"

#include "lua.h"
#include "lualib.h"

using namespace frostbyte;

int handleRecordOption(const char* option, const char*& arg, bool can_be_empty = false) {
    size_t option_length = strlen(option);

    if (strncmp(arg, option, option_length) != 0)
        return 1;

    if (strlen(arg) == option_length || arg[option_length] != '=') {
        fprintf(stderr, "ERROR: %s expects an equals sign\n", option);
        return 1;
    } else if (!can_be_empty && strlen(arg) < option_length + 2) {
        fprintf(stderr, "ERROR: %s expects a value after the equals sign\n", option);
        return 1;
    }

    arg += option_length + 1;
    return 0;
}

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

void writeStringToFile(const char* file_path, std::string_view contents) {
    std::ofstream file(file_path);

    file << contents;
}

size_t next_script_editor_tab_index = 0;
struct ScriptEditorTab {
    bool exists;
    bool newly_created;
    const ThreadIdentity* identity;
    ScriptLanguage* language;
    std::string name;
    std::string code;
};

std::vector<ScriptEditorTab> script_editor_tab_list;
void pushNewScriptEditorTab(std::string contents) {
    next_script_editor_tab_index++;

    std::string name = "script";
    name.append(std::to_string(next_script_editor_tab_index));

    script_editor_tab_list.push_back({
        .exists = true,
        .newly_created = true,
        .identity = &ThreadIdentity::GAME_SCRIPT,
        .language = &ScriptLanguage::Luau,
        .name = name,
        .code = contents
    });
}
void pushNewScriptEditorTab() {
    pushNewScriptEditorTab("print'frostbyte on top'");
}

std::string script_editor_save_path;
std::string script_editor_save_contents;
std::string_view script_editor_current_contents;

void tryRunCode(lua_State* L, const char* name, const char* code, size_t code_length, ScriptLanguage* language = nullptr, const ThreadIdentity* identity = nullptr) {
    try {
        TaskScheduler::startCodeOnNewThread(L, name, code, code_length, language, identity, [] (std::string error) {
            Console::ScriptConsole.error(error);
        });
    } catch(std::exception& e) {
        Console::ScriptConsole.error(e.what());
    }
}

Shader frostbyte::round_shader;

bool frostbyte::game_active = false;

int main(int argc, char** argv) {
    const double initial_game_time = lua_clock();
    TaskScheduler::initial_client_time = initial_game_time;

    if (argc < 1) {
        displayHelp();
        return 1;
    }

    for (unsigned i = 1; i < (unsigned) argc; i++) {
        const char* arg = argv[i];
        if (strequal(arg, "-h") || strequal(arg, "--help")) {
            displayHelp();
            return 0;
        } else if (strequal(arg, "--nosandbox"))
            TaskScheduler::sandboxing = false;
        else {
            fprintf(stderr, "ERROR: unrecognized option '%s'\n", arg);
            return 1;
        }
    }

    {
    const char* user_home = getenv("HOME");
    if (user_home == NULL) {
        fprintf(stderr, "ERROR: failed to get HOME environment variable\n");
        return 1;
    }

    FileSystem::home_path = std::string(user_home);
    FileSystem::home_path.append("/frostbyte/");

    FileSystem::workspace_path.assign(FileSystem::home_path);
    FileSystem::workspace_path.append("workspace/");

    FileSystem::bin_path.assign(FileSystem::home_path);
    FileSystem::bin_path.append("bin/");

    FileSystem::temp_path.assign(FileSystem::home_path);
    FileSystem::temp_path.append("temp/");

    if (!std::filesystem::exists(FileSystem::home_path) && !std::filesystem::create_directory(FileSystem::home_path)) {
        fprintf(stderr, "ERROR: failed to create folder at $HOME/frostbyte\n");
        return 1;
    }
    if (!std::filesystem::exists(FileSystem::workspace_path) && !std::filesystem::create_directory(FileSystem::workspace_path)) {
        fprintf(stderr, "ERROR: failed to create folder at $HOME/frostbyte/workspace\n");
        return 1;
    }
    if (!std::filesystem::exists(FileSystem::bin_path) && !std::filesystem::create_directory(FileSystem::bin_path)) {
        fprintf(stderr, "ERROR: failed to create folder at $HOME/frostbyte/bin\n");
        return 1;
    }
    if (!std::filesystem::exists(FileSystem::temp_path) && !std::filesystem::create_directory(FileSystem::temp_path)) {
        fprintf(stderr, "ERROR: failed to create folder at $HOME/frostbyte/temp\n");
        return 1;
    }

    ScriptLanguage::refresh();

    // FIXME: we need to pull assets from the repo if the assets folder doesn't exist!

    }

    std::string api_dump;
    try {
        std::string path = FileSystem::home_path;
        path.append("assets/Full-API-Dump.json");
        api_dump.assign(readFileToString(path.c_str()));
    } catch (std::exception& e) {
        fprintf(stderr, "ERROR: failed to read $HOME/frostbyte/assets/Full-API-Dump.json: %s\n", e.what());
        return 1;
    }

    curl_global_init(CURL_GLOBAL_DEFAULT);

    lua_State* L = luaL_newstate();
    luaL_openlibs(L);

    TaskScheduler::setup(L);

    Console::ScriptConsole.debugf("global state: %p", L->global);
    Console::ScriptConsole.debugf("main state: %p", L);

    lua_createtable(L, 0, 1);
    lua_pushstring(L, "kvs");
    lua_setfield(L, -2, "__mode");
    lua_setfield(L, LUA_REGISTRYINDEX, "weakmetatable");

    open_frostbyte_environment(L);

    SetTraceLogLevel(LOG_WARNING);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(800, 600, "frostbyte");
    SetExitKey(KEY_NULL);
    SetTargetFPS(TaskScheduler::target_fps);

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

    lua_State* appL = TaskScheduler::newThread(L, [] (std::string error) { Console::ScriptConsole.error(error); });
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

    lua_State* userL = TaskScheduler::newThread(L, [] (std::string error) { Console::ScriptConsole.error(error); });
    lua_pop(L, 1);
    Console::ScriptConsole.debugf("user state: %p", userL);

    lua_State* testL = TaskScheduler::newThread(L, [] (std::string error) { Console::TestsConsole.error(error); });
    lua_pop(L, 1);
    Console::ScriptConsole.debugf("test state: %p", testL);

    lua_State* fontL = TaskScheduler::newThread(L, [] (std::string error) { Console::ScriptConsole.error(error); });
    lua_pop(L, 1);
    FontLoader::L = fontL;
    Console::ScriptConsole.debugf("font state: %p", fontL);

    {
        char buf[100];
        snprintf(buf, 100, "App State (%p)", appL);
        getTask(appL)->identifier.assign(buf);
        snprintf(buf, 100, "User State (%p)", userL);
        getTask(userL)->identifier.assign(buf);
        snprintf(buf, 100, "Test State (%p)", testL);
        getTask(testL)->identifier.assign(buf);
        snprintf(buf, 100, "Font State (%p)", fontL);
        getTask(fontL)->identifier.assign(buf);
    }

    lua_State* protected_thread_list[] = { appL, userL, testL, fontL };
    size_t protected_thread_count = sizeof(protected_thread_list) / sizeof(protected_thread_list[0]);
    lua_State** protected_thread_list_end = protected_thread_list + protected_thread_count;

    bool all_tests_succeeded = false;
    bool has_tested = false;
    bool is_running_tests = false;
    bool should_run_tests = false;

    setupTests(&is_running_tests, &all_tests_succeeded);

    pushNewScriptEditorTab();

    {
        std::string base_path = FileSystem::home_path;
        base_path.append("assets/base.vs");
        std::string rounded_path = FileSystem::home_path;
        rounded_path.append("assets/rounded_rectangle.fs");

        round_shader = LoadShader(base_path.c_str(), rounded_path.c_str());

        if (!IsShaderValid(round_shader)) {
            fprintf(stderr, "ERROR: failed to load shaders at %s and %s\n", base_path.c_str(), rounded_path.c_str());
            return 1;
        }
    }
    float shader_zero = 0.f;

    {
        float vec4[] = { 5.f, 5.f, 5.f, 5.f };
        float vec2[] = { 0.f, 0.f };
        SetShaderValue(round_shader, GetShaderLocation(round_shader, "radius"), vec4, SHADER_UNIFORM_VEC4);
        SetShaderValue(round_shader, GetShaderLocation(round_shader, "shadowRadius"), &shader_zero, SHADER_UNIFORM_FLOAT);
        SetShaderValue(round_shader, GetShaderLocation(round_shader, "shadowOffset"), vec2, SHADER_UNIFORM_VEC2);
        SetShaderValue(round_shader, GetShaderLocation(round_shader, "shadowScale"), &shader_zero, SHADER_UNIFORM_FLOAT);
        SetShaderValue(round_shader, GetShaderLocation(round_shader, "borderThickness"), &shader_zero, SHADER_UNIFORM_FLOAT);
    }

    rlImGuiSetup(true);

    default_imgui_style = ImGui::GetStyle();
    changeImGuiTheme(imgui_theme);

    DataModel::onLoad(L);

    game_active = true;
    while (!WindowShouldClose() && !DataModel::shutdown) {
        const bool anyImGui = ImGui::IsWindowHovered(ImGuiHoveredFlags_AnyWindow);
        if (enable_user_input_service)
            UserInputService::process(appL, anyImGui);
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

        BeginDrawing();
        ClearBackground(DARKGRAY);

        // gui object render
        if (enable_gui_object_rendering)
            rbxInstance_BasePlayerGui_render(appL, anyImGui);

        // lua drawings
        DrawEntry::render();
        render_drawingimmediate(appL);

        // ui
        rlImGuiBegin();
        const float imgui_frame_height = ImGui::GetFrameHeightWithSpacing();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Options")) {
                ImGui::BeginDisabled();
                ImGui::Text("sandboxing - %s", TaskScheduler::sandboxing ? "enabled" : "disabled");
                ImGui::EndDisabled();

                ImGui::MenuItem("print routes to stdout", nullptr, &print_stdout);
                if (ImGui::MenuItem("enable stephook", nullptr, &enable_stephook))
                    onEnableStephookChange(appL);

                ImGui::MenuItem("RunService IsServer", nullptr, &runservice_is_server);
                ImGui::MenuItem("RunService IsStudio", nullptr, &runservice_is_studio);

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Theme")) {
                if (ImGui::MenuItem("Monochrome", nullptr, imgui_theme == ImGuiTheme::Monochrome))
                    changeImGuiTheme(ImGuiTheme::Monochrome);
                if (ImGui::MenuItem("SynV2", nullptr, imgui_theme == ImGuiTheme::SynV2))
                    changeImGuiTheme(ImGuiTheme::SynV2);
                if (ImGui::MenuItem("CatppuccinMocha", nullptr, imgui_theme == ImGuiTheme::CatppuccinMocha))
                    changeImGuiTheme(ImGuiTheme::CatppuccinMocha);
                if (ImGui::MenuItem("Gold", nullptr, imgui_theme == ImGuiTheme::Gold))
                    changeImGuiTheme(ImGuiTheme::Gold);
                if (ImGui::MenuItem("SonicRiders", nullptr, imgui_theme == ImGuiTheme::SonicRiders))
                    changeImGuiTheme(ImGuiTheme::SonicRiders);
                if (ImGui::MenuItem("ClassicSteam", nullptr, imgui_theme == ImGuiTheme::ClassicSteam))
                    changeImGuiTheme(ImGuiTheme::ClassicSteam);

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Window")) {
                ImGui::MenuItem("Show FPS", nullptr, &show_fps);
                ImGui::Separator();
                ImGui::MenuItem("Script Editor", nullptr, &menu_editor_open);
                ImGui::MenuItem("Script Console", nullptr, &menu_console_open);
                ImGui::Separator();
                ImGui::MenuItem("Tests", nullptr, &menu_tests_open);
                ImGui::Separator();
                ImGui::MenuItem("Thread List", nullptr, &menu_thread_list_open);
                ImGui::MenuItem("DrawEntry List", nullptr, &menu_drawentry_list_open);
                ImGui::Separator();
                ImGui::MenuItem("Instance Explorer", nullptr, &menu_instance_explorer_open);

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debugging")) {
                ImGui::MenuItem("Enable UserInputService", nullptr, &enable_user_input_service);
                ImGui::MenuItem("Enable RunService", nullptr, &enable_run_service);
                ImGui::MenuItem("Enable TweenService", nullptr, &enable_tween_service);
                ImGui::MenuItem("Enable GuiObject rendering", nullptr, &enable_gui_object_rendering);

                ImGui::MenuItem("HttpGet synchronous Argument Enabled", nullptr, &httpget_synchronous_argument);

                ImGui::MenuItem("Function Explorer", nullptr, &menu_function_explorer_open);
                ImGui::MenuItem("Table Explorer", nullptr, &menu_table_explorer_open);
                ImGui::MenuItem("Image Explorer", nullptr, &menu_image_explorer_open);
                ImGui::MenuItem("Font Explorer", nullptr, &menu_font_explorer_open);

                ImGui::EndMenu();
            }

            // ImGui::Text("CPU Usage: %f%%", SysUtils::cpu_percent_used);
            ImGui::Text("RAM Usage: %.2f GB / %.2f GB (%f%%)", SysUtils::physical_memory_used, SysUtils::physical_memory_total, SysUtils::physical_memory_used / SysUtils::physical_memory_total * 100.0);

            menu_bar_height = ImGui::GetFrameHeight();
            ImGui::EndMainMenuBar();
        }

        if (menu_editor_open) {
            if (ImGui::Begin("Script Editor", &menu_editor_open)) {
                if (ImGui::Button("Open File(s)")) {
                    IGFD::FileDialogConfig config;
                    config.countSelectionMax = 0;
                    config.path = ".";
                    // TODO: .moon and .clue and then check extension to automatically set tab language
                    ImGuiFileDialog::Instance()->OpenDialog("scripteditoropen", "Open File(s)", ".luau,.lua,.*", config); 
                }
                ImGui::SameLine();
                if (ImGui::Button("Execute File(s)")) {
                    IGFD::FileDialogConfig config;
                    config.countSelectionMax = 0;
                    config.path = ".";
                    // TODO: .moon and .clue and then check extension to convert
                    ImGuiFileDialog::Instance()->OpenDialog("scripteditorexecute", "Execute File(s)", ".luau,.lua,.*", config); 
                }
                ImGui::SameLine();
                if (ImGui::Button("Save To File")) {
                    IGFD::FileDialogConfig config;
                    config.path = ".";
                    ImGuiFileDialog::Instance()->OpenDialog("scripteditorsave", "Save File", ".*", config); 
                }

                ImGui::Separator();

                if (ImGuiFileDialog::Instance()->Display("scripteditoropen", ImGuiWindowFlags_NoCollapse, ImVec2{0, 250})) {
                    if (ImGuiFileDialog::Instance()->IsOk())
                        for (auto& pair : ImGuiFileDialog::Instance()->GetSelection())
                            pushNewScriptEditorTab(readFileToString(pair.second.c_str()));

                    ImGuiFileDialog::Instance()->Close();
                }
                if (ImGuiFileDialog::Instance()->Display("scripteditorexecute", ImGuiWindowFlags_NoCollapse, ImVec2{0, 250})) {
                    if (ImGuiFileDialog::Instance()->IsOk())
                        for (auto& pair : ImGuiFileDialog::Instance()->GetSelection()) {
                            std::string contents = readFileToString(pair.second.c_str());
                            tryRunCode(userL, pair.first.c_str(), contents.c_str(), contents.length());
                        }

                    ImGuiFileDialog::Instance()->Close();
                }
                if (ImGuiFileDialog::Instance()->Display("scripteditorsave", ImGuiWindowFlags_NoCollapse, ImVec2{0, 250})) {
                    if (ImGuiFileDialog::Instance()->IsOk()) {
                        std::string file_path = ImGuiFileDialog::Instance()->GetFilePathName();

                        if (ImGuiFileDialog::Instance()->GetSelection().empty())
                            // TODO: error handling & feedback
                            writeStringToFile(file_path.c_str(), script_editor_current_contents);
                        else {
                            script_editor_save_path = file_path;
                            script_editor_save_contents = script_editor_current_contents;
                        }

                        Console::ScriptConsole.debugf("file path: %s", file_path.c_str());
                    }

                    ImGuiFileDialog::Instance()->Close();
                }

                if (!script_editor_save_path.empty()) {
                    if (ImGui::Begin("Save File Confirmation")) {
                        ImGui::Text("Are you sure you want to overwrite '%s'?", script_editor_save_path.c_str());
                        const bool yes = ImGui::Button("YES");
                        const bool no = ImGui::Button("NO");

                        // TODO: feedback
                        if (yes)
                            // TODO: error handling
                            writeStringToFile(script_editor_save_path.c_str(), script_editor_save_contents);

                        if (yes || no)
                            script_editor_save_path.clear();

                        ImGui::End();
                    }
                }

                if (ImGui::Button("+##scripteditornewtab"))
                    pushNewScriptEditorTab();

                ImGui::SameLine();

                if (ImGui::BeginTabBar("scripteditortabs", ImGuiTabBarFlags_Reorderable )) {
                    for (size_t i = 0; i < script_editor_tab_list.size(); i++) {
                        auto& tab = script_editor_tab_list[i];

                        ImGuiTabItemFlags flags = 0;
                        if (tab.newly_created) {
                            flags |= ImGuiTabItemFlags_SetSelected;
                            tab.newly_created = false;
                        }
                        if (ImGui::BeginTabItem(tab.name.c_str(), &tab.exists, flags)) {
                            ImGui_ScriptLanguageCombo(&tab.language);

                            ImGui::InputTextMultiline(
                                "##scriptbox",
                                &tab.code[0],
                                tab.code.capacity() + 1,
                                ImVec2(-FLT_MIN, ImGui::GetContentRegionAvail().y - imgui_frame_height * 3.f),
                                ImGuiInputTextFlags_CallbackResize,
                                imgui_inputTextCallback,
                                (void*) &tab.code
                            );
                            script_editor_current_contents = tab.code;

                            int id = tab.identity->id;
                            if (ImGui_ThreadIdentityCombo(&id))
                                tab.identity = identity_map.at(id);

                            if (ImGui::Button("Execute"))
                                tryRunCode(userL, tab.name.c_str(), tab.code.c_str(), tab.code.length(), tab.language, tab.identity);

                            ImGui::SameLine();
                            if (ImGui::Button("Clear"))
                                tab.code.clear();

                            ImGui::SameLine();
                            ImGui::Text("%s", "");
                            ImGui::SameLine();
                            if (ImGui::Button("Execute Clipboard")) {
                                const char* text = GetClipboardText();
                                if (text)
                                    tryRunCode(userL, tab.name.c_str(), text, strlen(text), tab.language, tab.identity);
                                else
                                    Console::ScriptConsole.warning("Failed to get clipboard contents");
                            }

                            ImGui::EndTabItem();
                        }
                    }
                    // remove tabs that were closed
                    for (auto tab = script_editor_tab_list.begin(); tab != script_editor_tab_list.end();) {
                        if (!tab->exists)
                            tab = script_editor_tab_list.erase(tab);
                        else
                            ++tab;
                    }
                    if (script_editor_tab_list.empty())
                        pushNewScriptEditorTab();
                    ImGui::EndTabBar();
                }

            }
            ImGui::End();
        }
        if (menu_console_open) {
            if (ImGui::Begin("Script Console", &menu_console_open)) {
                const bool clear_button = ImGui::Button("Clear");
                ImGui::SameLine();
                const bool copytoclipboard_button = ImGui::Button("Copy Contents");

                if (clear_button)
                    Console::ScriptConsole.clear();
                else if (copytoclipboard_button)
                    SetClipboardText(Console::ScriptConsole.getWholeContent().c_str());
                else {
                    static const char* goto_text = "go to";
                    static const char* top_button_text = "top";
                    static const char* bottom_button_text = "bottom";
                    float text_width = ImGui::CalcTextSize(goto_text).x + ImGui::GetStyle().FramePadding.x * 2.f;
                    float top_button_width = ImGui::CalcTextSize(top_button_text).x + ImGui::GetStyle().FramePadding.x * 2.f;
                    float bottom_button_width = ImGui::CalcTextSize(bottom_button_text).x + ImGui::GetStyle().FramePadding.x * 2.f;

                    ImGui::SameLine();
                    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - (text_width + ImGui::GetStyle().ItemSpacing.x + top_button_width + bottom_button_width));
                    ImGui::Text("%s", goto_text);
                    ImGui::SameLine();
                    bool go_to_top = ImGui::Button(top_button_text);
                    ImGui::SameLine();
                    bool go_to_bottom = ImGui::Button(bottom_button_text);

                    ImGui::Checkbox("##info", &Console::ScriptConsole.show_info);
                    ImGui::SameLine();
                    ImGui::TextColored(Console::ColorINFO, "INFO");
                    if (ImGui::IsItemClicked())
                        Console::ScriptConsole.show_info ^= true;
                    ImGui::SameLine();

                    ImGui::Checkbox("##warning", &Console::ScriptConsole.show_warning);
                    ImGui::SameLine();
                    ImGui::TextColored(Console::ColorWARNING, "WARNING");
                    if (ImGui::IsItemClicked())
                        Console::ScriptConsole.show_warning ^= true;
                    ImGui::SameLine();

                    ImGui::Checkbox("##error", &Console::ScriptConsole.show_error);
                    ImGui::SameLine();
                    ImGui::TextColored(Console::ColorERROR, "ERROR");
                    if (ImGui::IsItemClicked())
                        Console::ScriptConsole.show_error ^= true;
                    ImGui::SameLine();

                    ImGui::Checkbox("##debug", &Console::ScriptConsole.show_debug);
                    ImGui::SameLine();
                    ImGui::TextColored(Console::ColorDEBUG, "DEBUG");
                    if (ImGui::IsItemClicked())
                        Console::ScriptConsole.show_debug ^= true;

                    ImGui::Separator();

                    ImGui::BeginChild("ScrollableRegion", ImGui::GetContentRegionAvail(), 0,  ImGuiWindowFlags_HorizontalScrollbar);
                    if (!go_to_bottom) go_to_bottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY();

                    Console::ScriptConsole.renderMessages();

                    if (go_to_top)
                        ImGui::SetScrollY(0.f);
                    else if (go_to_bottom)
                        ImGui::SetScrollHereY(1.f);

                    ImGui::EndChild();
                }
            }
            ImGui::End();
        }
        if (menu_tests_open) {
            if (ImGui::Begin("Tests", &menu_tests_open)) {
                if (is_running_tests)
                    ImGui::Text("Running tests...");
                else {
                    if (ImGui::Button("Run all tests")) {
                        has_tested = true;
                        is_running_tests = true;
                        should_run_tests = true;
                        Console::TestsConsole.clear();
                    } else if (has_tested) {
                        if (all_tests_succeeded)
                            ImGui::TextColored({0.4, 1, 0.4, 1}, "All tests succeeded!");
                        else
                            ImGui::TextColored({1, 0.4, 0.4, 1}, "Some tests failed!");
                    }
                }

                ImGui::BeginChild("ScrollableRegion", ImGui::GetContentRegionAvail(), 0,  ImGuiWindowFlags_HorizontalScrollbar);
                Console::TestsConsole.renderMessages();
                ImGui::EndChild();

            }
            ImGui::End();
        }
        if (menu_thread_list_open) {
            if (ImGui::Begin("Thread List", &menu_thread_list_open)) {
                lua_State* thread_to_kill = nullptr;

                // std::shared_lock lock(TaskScheduler::thread_list_mutex);
                for (size_t i = 0; i < TaskScheduler::thread_list.size(); i++) {
                    lua_State* thread = TaskScheduler::thread_list[i];
                    Task* task = getTask(thread);
                    std::string identifier = task->identifier;

                    if (ImGui::Button(identifier.c_str()))
                        task->view.open = true;

                    if (i == protected_thread_count - 1) {
                        ImGui::Separator();
                        ImGui::Text("Thread count: %lu", TaskScheduler::thread_list.size() - protected_thread_count);
                    }

                    if (task->view.open) {
                        std::string win_id = std::string("Thread ");
                        win_id.append(identifier);
                        if (ImGui::Begin(win_id.c_str(), &task->view.open)) {
                            static const char* status_item_list[] = { "Idle", "Running", "Yielding", "Waiting", "Deferring", "Delaying" };
                            ImGui::Combo("Status", reinterpret_cast<int*>(&task->status), status_item_list, IM_ARRAYSIZE(status_item_list));

                            int id = task->identity->id;
                            if (ImGui_ThreadIdentityCombo(&id))
                                task->identity = identity_map.at(id);

                            const bool is_protected = std::find(protected_thread_list, protected_thread_list_end, thread) != protected_thread_list_end;
                            if (is_protected)
                                ImGui::BeginDisabled();

                            if (ImGui::Button("Kill"))
                                thread_to_kill = thread;

                            if (is_protected) {
                                ImGui::SetItemTooltip("protected thread");
                                ImGui::EndDisabled();
                            }

                            ImGui::End();
                        }
                    }
                }
                // lock.unlock();

                if (thread_to_kill)
                    TaskScheduler::killThread(thread_to_kill);
            }
            ImGui::End();
        }
        if (menu_drawentry_list_open) {
            if (ImGui::Begin("DrawEntry List", &menu_drawentry_list_open, ImGuiWindowFlags_MenuBar))
                UI_DrawEntryList_render(appL);
            ImGui::End();
        }
        if (menu_instance_explorer_open) {
            if (ImGui::Begin("Instance Explorer", &menu_instance_explorer_open, ImGuiWindowFlags_MenuBar))
                UI_InstanceExplorer_render(appL);
            ImGui::End();
        }

        if (menu_function_explorer_open) {
            if (ImGui::Begin("Function Explorer", &menu_function_explorer_open, ImGuiWindowFlags_MenuBar))
                UI_FunctionExplorer_render(appL);
            ImGui::End();
        }
        if (menu_table_explorer_open) {
            if (ImGui::Begin("Table Explorer", &menu_table_explorer_open, ImGuiWindowFlags_MenuBar))
                UI_TableExplorer_render(appL);
            ImGui::End();
        }
        if (menu_image_explorer_open) {
            if (ImGui::Begin("Image Explorer", &menu_image_explorer_open))
                UI_ImageExplorer_render(appL);
            ImGui::End();
        }
        if (menu_font_explorer_open) {
            if (ImGui::Begin("Font Explorer", &menu_font_explorer_open))
                UI_FontExplorer_render(appL);
            ImGui::End();
        }

        ImGuiService_render(appL);

        if (show_fps)
            DrawFPS(30, 30);

        rlImGuiEnd();

        EndDrawing();

        if (should_run_tests) {
            should_run_tests = false;
            startAllTests(testL);
        }

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

        setInstanceValue<double>(Workspace::instance, appL, "DistributedGameTime", lua_clock() - initial_game_time);

        SysUtils::run();
    }
    DataModel::onShutdown(appL);
    game_active = false;

    UnloadShader(round_shader);

    for (auto& entry : DrawEntry::draw_list)
        entry->free();

    rlImGuiShutdown();
    FontLoader::unload();
    UI_ImageExplorer_cleanup();
    ImageLoader::unload();
    CloseWindow();

    rbxInstanceCleanup(appL);
    rbxScriptSignal::cleanup();

    // for (int i = 0; i < IM_ARRAYSIZE(protected_thread_list); i++)
    //     TaskScheduler::killThread(protected_thread_list[i]);

    // probably redundant?
    TaskScheduler::cleanup();

    lua_close(L);

    curl_global_cleanup();

    return 0;
}
