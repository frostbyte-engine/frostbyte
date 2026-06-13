#include <cstdio>
#include <cstring>

// us
#include "imgui.h"
#include "imguitheme.hpp"

// frostbyte
#include "frostbyte.hpp"

// frostbyte /
#include "fontloader.hpp"
#include "taskscheduler.hpp"
#include "tests.hpp"
#include "sysutils.hpp"

// frostbyte / ui
#include "ui/drawentrylist.hpp"
#include "ui/instanceexplorer.hpp"
#include "ui/functionexplorer.hpp"
#include "ui/tableexplorer.hpp"
#include "ui/imageexplorer.hpp"
#include "ui/fontexplorer.hpp"
#include "ui/ui.hpp"

// frostbyte / libraries
#include "libraries/instructionlib.hpp"

// frostbyte / engine / classes
#include "engine/classes/datamodel.hpp"
#include "engine/classes/frostbyte/imguiservice.hpp"

// external
#include "raylib.h"
#include "rlImGui.h"
#include "ImGuiFileDialog.h"

#define strequal(str1, str2) (strcmp(str1, str2) == 0)

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

void displayHelp(const char* filename = "frostbyte") {
    printf("frostbyte by techhog\n"
        "usage: %s [options]\n\n"
        "options:\n"
        "  -h           -  displays this page\n"
        "  --nosandbox  -  disables sandboxing (Luau will perform less optimizations, but functions like getgenv need this flag to work)\n"
    , filename);
}

void writeStringToFile(const char* file_path, std::string_view contents) {
    std::ofstream file(file_path);

    file << contents;
}

size_t next_script_editor_tab_index = 0;
struct ScriptEditorTab {
    bool exists;
    bool newly_created;
    const frostbyte::ThreadIdentity* identity;
    frostbyte::ScriptLanguage* language;
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
        .identity = &frostbyte::ThreadIdentity::GAME_SCRIPT,
        .language = &frostbyte::ScriptLanguage::Luau,
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

void tryRunCode(lua_State* L, const char* name, const char* code, size_t code_length, frostbyte::ScriptLanguage* language = nullptr, const frostbyte::ThreadIdentity* identity = nullptr) {
    try {
        frostbyte::TaskScheduler::startCodeOnNewThread(L, name, code, code_length, language, identity, [] (std::string error) {
            frostbyte::Console::ScriptConsole.error(error);
        });
    } catch(std::exception& e) {
        frostbyte::Console::ScriptConsole.error(e.what());
    }
}

bool app(frostbyte::FrostbyteConfiguration& configuration) {
    try {
        frostbyte::Frostbyte::initialize(configuration);
    } catch (std::exception& e) {
        fprintf(stderr, "ERROR: failed to initialize frostbyte: %s", e.what());
        exit(1);
    }

    lua_State* L = frostbyte::Frostbyte::L;
    lua_State* appL = frostbyte::Frostbyte::appL;

    lua_State* userL = frostbyte::TaskScheduler::newThread(L, [] (std::string error) { frostbyte::Console::ScriptConsole.error(error); });
    lua_pop(L, 1);
    frostbyte::Console::ScriptConsole.debugf("user state: %p", userL);

    lua_State* testL = frostbyte::TaskScheduler::newThread(L, [] (std::string error) { frostbyte::Console::TestsConsole.error(error); });
    lua_pop(L, 1);
    frostbyte::Console::ScriptConsole.debugf("test state: %p", testL);

    lua_State* fontL = frostbyte::TaskScheduler::newThread(L, [] (std::string error) { frostbyte::Console::ScriptConsole.error(error); });
    lua_pop(L, 1);
    frostbyte::FontLoader::L = fontL;
    frostbyte::Console::ScriptConsole.debugf("font state: %p", fontL);

    {
        char buf[100];
        snprintf(buf, 100, "App State (%p)", appL);
        frostbyte::getTask(appL)->identifier.assign(buf);
        snprintf(buf, 100, "User State (%p)", userL);
        frostbyte::getTask(userL)->identifier.assign(buf);
        snprintf(buf, 100, "Test State (%p)", testL);
        frostbyte::getTask(testL)->identifier.assign(buf);
        snprintf(buf, 100, "Font State (%p)", fontL);
        frostbyte::getTask(fontL)->identifier.assign(buf);
    }

    lua_State* protected_thread_list[] = { appL, userL, testL, fontL };
    size_t protected_thread_count = sizeof(protected_thread_list) / sizeof(protected_thread_list[0]);
    lua_State** protected_thread_list_end = protected_thread_list + protected_thread_count;

    bool all_tests_succeeded = false;
    bool has_tested = false;
    bool is_running_tests = false;
    bool should_run_tests = false;

    frostbyte::setupTests(&is_running_tests, &all_tests_succeeded);

    bool close = false;
    bool restart = false;

    while (!close) {
        frostbyte::Frostbyte::preRender();

        frostbyte::Frostbyte::beginRender();

        rlImGuiBegin();

        const float imgui_frame_height = ImGui::GetFrameHeightWithSpacing();

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("Options")) {
                ImGui::BeginDisabled();
                ImGui::Text("sandboxing - %s", frostbyte::TaskScheduler::sandboxing ? "enabled" : "disabled");
                ImGui::EndDisabled();

                ImGui::MenuItem("print routes to stdout", nullptr, &frostbyte::print_stdout);
                if (ImGui::MenuItem("enable stephook", nullptr, &frostbyte::enable_stephook))
                    frostbyte::onEnableStephookChange(frostbyte::Frostbyte::appL);

                ImGui::MenuItem("RunService IsServer", nullptr, &frostbyte::runservice_is_server);
                ImGui::MenuItem("RunService IsStudio", nullptr, &frostbyte::runservice_is_studio);

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
                ImGui::MenuItem("Show FPS", nullptr, &frostbyte::show_fps);
                ImGui::Separator();
                ImGui::MenuItem("Script Editor", nullptr, &frostbyte::menu_editor_open);
                ImGui::MenuItem("Script Console", nullptr, &frostbyte::menu_console_open);
                ImGui::Separator();
                ImGui::MenuItem("Tests", nullptr, &frostbyte::menu_tests_open);
                ImGui::Separator();
                ImGui::MenuItem("Thread List", nullptr, &frostbyte::menu_thread_list_open);
                ImGui::MenuItem("DrawEntry List", nullptr, &frostbyte::menu_drawentry_list_open);
                ImGui::Separator();
                ImGui::MenuItem("Instance Explorer", nullptr, &frostbyte::menu_instance_explorer_open);

                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Debugging")) {
                ImGui::BeginDisabled();
                if (ImGui::Button("Restart Game"))
                    restart = true;
                ImGui::EndDisabled();

                ImGui::MenuItem("Enable UserInputService", nullptr, &frostbyte::enable_user_input_service);
                ImGui::MenuItem("Enable RunService", nullptr, &frostbyte::enable_run_service);
                ImGui::MenuItem("Enable TweenService", nullptr, &frostbyte::enable_tween_service);
                ImGui::MenuItem("Enable GuiObject rendering", nullptr, &frostbyte::enable_gui_object_rendering);

                ImGui::MenuItem("HttpGet synchronous Argument Enabled", nullptr, &frostbyte::httpget_synchronous_argument);

                ImGui::MenuItem("Function Explorer", nullptr, &frostbyte::menu_function_explorer_open);
                ImGui::MenuItem("Table Explorer", nullptr, &frostbyte::menu_table_explorer_open);
                ImGui::MenuItem("Image Explorer", nullptr, &frostbyte::menu_image_explorer_open);
                ImGui::MenuItem("Font Explorer", nullptr, &frostbyte::menu_font_explorer_open);

                ImGui::EndMenu();
            }

            // ImGui::Text("CPU Usage: %f%%", SysUtils::cpu_percent_used);
            ImGui::Text("RAM Usage: %.2f GB / %.2f GB (%f%%)", frostbyte::SysUtils::physical_memory_used, frostbyte::SysUtils::physical_memory_total, frostbyte::SysUtils::physical_memory_used / frostbyte::SysUtils::physical_memory_total * 100.0);

            frostbyte::menu_bar_height = ImGui::GetFrameHeight();
            ImGui::EndMainMenuBar();
        }

        if (frostbyte::menu_editor_open) {
            if (ImGui::Begin("Script Editor", &frostbyte::menu_editor_open)) {
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

                        frostbyte::Console::ScriptConsole.debugf("file path: %s", file_path.c_str());
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
                                frostbyte::imgui_inputTextCallback,
                                (void*) &tab.code
                            );
                            script_editor_current_contents = tab.code;

                            int id = tab.identity->id;
                            if (frostbyte::ImGui_ThreadIdentityCombo(&id))
                                tab.identity = frostbyte::identity_map.at(id);

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
                                    frostbyte::Console::ScriptConsole.warning("Failed to get clipboard contents");
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
        if (frostbyte::menu_console_open) {
            if (ImGui::Begin("Script Console", &frostbyte::menu_console_open)) {
                const bool clear_button = ImGui::Button("Clear");
                ImGui::SameLine();
                const bool copytoclipboard_button = ImGui::Button("Copy Contents");

                if (clear_button)
                    frostbyte::Console::ScriptConsole.clear();
                else if (copytoclipboard_button)
                    SetClipboardText(frostbyte::Console::ScriptConsole.getWholeContent().c_str());
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

                    ImGui::Checkbox("##info", &frostbyte::Console::ScriptConsole.show_info);
                    ImGui::SameLine();
                    ImGui::TextColored(frostbyte::Console::ColorINFO, "INFO");
                    if (ImGui::IsItemClicked())
                        frostbyte::Console::ScriptConsole.show_info ^= true;
                    ImGui::SameLine();

                    ImGui::Checkbox("##warning", &frostbyte::Console::ScriptConsole.show_warning);
                    ImGui::SameLine();
                    ImGui::TextColored(frostbyte::Console::ColorWARNING, "WARNING");
                    if (ImGui::IsItemClicked())
                        frostbyte::Console::ScriptConsole.show_warning ^= true;
                    ImGui::SameLine();

                    ImGui::Checkbox("##error", &frostbyte::Console::ScriptConsole.show_error);
                    ImGui::SameLine();
                    ImGui::TextColored(frostbyte::Console::ColorERROR, "ERROR");
                    if (ImGui::IsItemClicked())
                        frostbyte::Console::ScriptConsole.show_error ^= true;
                    ImGui::SameLine();

                    ImGui::Checkbox("##debug", &frostbyte::Console::ScriptConsole.show_debug);
                    ImGui::SameLine();
                    ImGui::TextColored(frostbyte::Console::ColorDEBUG, "DEBUG");
                    if (ImGui::IsItemClicked())
                        frostbyte::Console::ScriptConsole.show_debug ^= true;

                    ImGui::Separator();

                    ImGui::BeginChild("ScrollableRegion", ImGui::GetContentRegionAvail(), 0,  ImGuiWindowFlags_HorizontalScrollbar);
                    if (!go_to_bottom) go_to_bottom = ImGui::GetScrollY() >= ImGui::GetScrollMaxY();

                    frostbyte::Console::ScriptConsole.renderMessages();

                    if (go_to_top)
                        ImGui::SetScrollY(0.f);
                    else if (go_to_bottom)
                        ImGui::SetScrollHereY(1.f);

                    ImGui::EndChild();
                }
            }
            ImGui::End();
        }
        if (frostbyte::menu_tests_open) {
            if (ImGui::Begin("Tests", &frostbyte::menu_tests_open)) {
                if (is_running_tests)
                    ImGui::Text("Running tests...");
                else {
                    if (ImGui::Button("Run all tests")) {
                        has_tested = true;
                        is_running_tests = true;
                        should_run_tests = true;
                        frostbyte::Console::TestsConsole.clear();
                    } else if (has_tested) {
                        if (all_tests_succeeded)
                            ImGui::TextColored({0.4, 1, 0.4, 1}, "All tests succeeded!");
                        else
                            ImGui::TextColored({1, 0.4, 0.4, 1}, "Some tests failed!");
                    }
                }

                ImGui::BeginChild("ScrollableRegion", ImGui::GetContentRegionAvail(), 0,  ImGuiWindowFlags_HorizontalScrollbar);
                frostbyte::Console::TestsConsole.renderMessages();
                ImGui::EndChild();

            }
            ImGui::End();
        }
        if (frostbyte::menu_thread_list_open) {
            if (ImGui::Begin("Thread List", &frostbyte::menu_thread_list_open)) {
                lua_State* thread_to_kill = nullptr;

                // std::shared_lock lock(TaskScheduler::thread_list_mutex);
                for (size_t i = 0; i < frostbyte::TaskScheduler::thread_list.size(); i++) {
                    lua_State* thread = frostbyte::TaskScheduler::thread_list[i];
                    frostbyte::Task* task = frostbyte::getTask(thread);
                    std::string identifier = task->identifier;

                    if (ImGui::Button(identifier.c_str()))
                        task->view.open = true;

                    if (i == protected_thread_count - 1) {
                        ImGui::Separator();
                        ImGui::Text("Thread count: %lu", frostbyte::TaskScheduler::thread_list.size() - protected_thread_count);
                    }

                    if (task->view.open) {
                        std::string win_id = std::string("Thread ");
                        win_id.append(identifier);
                        if (ImGui::Begin(win_id.c_str(), &task->view.open)) {
                            static const char* status_item_list[] = { "Idle", "Running", "Yielding", "Waiting", "Deferring", "Delaying" };
                            ImGui::Combo("Status", reinterpret_cast<int*>(&task->status), status_item_list, IM_ARRAYSIZE(status_item_list));

                            int id = task->identity->id;
                            if (frostbyte::ImGui_ThreadIdentityCombo(&id))
                                task->identity = frostbyte::identity_map.at(id);

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
                    frostbyte::TaskScheduler::killThread(thread_to_kill);
            }
            ImGui::End();
        }
        if (frostbyte::menu_drawentry_list_open) {
            if (ImGui::Begin("DrawEntry List", &frostbyte::menu_drawentry_list_open, ImGuiWindowFlags_MenuBar))
                frostbyte::UI_DrawEntryList_render(appL);
            ImGui::End();
        }
        if (frostbyte::menu_instance_explorer_open) {
            if (ImGui::Begin("Instance Explorer", &frostbyte::menu_instance_explorer_open, ImGuiWindowFlags_MenuBar))
                frostbyte::UI_InstanceExplorer_render(appL);
            ImGui::End();
        }

        if (frostbyte::menu_function_explorer_open) {
            if (ImGui::Begin("Function Explorer", &frostbyte::menu_function_explorer_open, ImGuiWindowFlags_MenuBar))
                frostbyte::UI_FunctionExplorer_render(appL);
            ImGui::End();
        }
        if (frostbyte::menu_table_explorer_open) {
            if (ImGui::Begin("Table Explorer", &frostbyte::menu_table_explorer_open, ImGuiWindowFlags_MenuBar))
                frostbyte::UI_TableExplorer_render(appL);
            ImGui::End();
        }
        if (frostbyte::menu_image_explorer_open) {
            if (ImGui::Begin("Image Explorer", &frostbyte::menu_image_explorer_open))
                frostbyte::UI_ImageExplorer_render(appL);
            ImGui::End();
        }
        if (frostbyte::menu_font_explorer_open) {
            if (ImGui::Begin("Font Explorer", &frostbyte::menu_font_explorer_open))
                frostbyte::UI_FontExplorer_render(appL);
            ImGui::End();
        }

        frostbyte::ImGuiService_render(appL);

        rlImGuiEnd();

        frostbyte::Frostbyte::endRender();

        if (should_run_tests) {
            should_run_tests = false;
            frostbyte::startAllTests(testL);
        }

        frostbyte::Frostbyte::postRender();

        // close = WindowShouldClose() || frostbyte::DataModel::shutdown || restart;
        close = WindowShouldClose() || frostbyte::DataModel::shutdown;
    }

    frostbyte::Frostbyte::cleanup(restart);

    return restart;
}

int main(int argc, char** argv) {
    if (argc < 1) {
        displayHelp();
        return 1;
    }

    frostbyte::FrostbyteConfiguration configuration;

    for (unsigned i = 1; i < (unsigned) argc; i++) {
        const char* arg = argv[i];
        if (strequal(arg, "-h") || strequal(arg, "--help")) {
            displayHelp();
            return 0;
        } else if (strequal(arg, "--nosandbox"))
            configuration.sandbox_enabled = false;
        else {
            fprintf(stderr, "ERROR: unrecognized option '%s'\n", arg);
            return 1;
        }
    }

    const char* user_home = getenv("HOME");
    if (user_home == NULL) {
        fprintf(stderr, "ERROR: failed to get HOME environment variable\n");
        return 1;
    }

    std::string home_path = std::string(user_home);
    home_path.append("/frostbyte/");

    configuration.home_path = home_path.c_str();

    configuration.initializeWindow = []() {
        SetTraceLogLevel(LOG_WARNING);
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
        InitWindow(800, 600, "frostbyte");
        SetExitKey(KEY_NULL);
        SetTargetFPS(frostbyte::TaskScheduler::target_fps);

        rlImGuiSetup(true);
    };
    configuration.cleanupWindow = []() {
        rlImGuiShutdown();

        CloseWindow();
    };

    while (true) {
        if (!app(configuration))
            break;
    }

    return 0;
}
