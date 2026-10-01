#include <atomic>
#include <csignal>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iostream>

// frostbyte
#include "frostbyte.hpp"

// frostbyte /
#include "interface.hpp"
#include "scriptlanguage.hpp"
#include "taskscheduler.hpp"

// frostbyte / engine / classes
#include "engine/classes/datamodel.hpp"

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
        throw std::runtime_error(std::string("failed to open file ").append(file_path));

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
    printf("frostbyte-server by techhog\n"
        "usage: %s [options]\n\n"
        "options:\n"
        "  -h                   -  displays this page\n"
        "  --nosandbox          -  disables sandboxing (Luau will perform less optimizations, but functions like getgenv need this flag to work)\n"
        "  --script=FILE        -  run a script from a file\n"
        "  --code=CODE          -  run a script CODE\n"
        "  --language=LANGUAGE  -  the language scripts will use\n"
        "\nlanguage choices (case sensitive):\n"
        "  luau (default)\n"
        "  moonscript\n"
        "  clue\n"
    , filename);
}

void writeStringToFile(const char* file_path, std::string_view contents) {
    std::ofstream file(file_path);

    file << contents;
}

void gameCycle() {
    frostbyte::Frostbyte::preRender();
    frostbyte::Frostbyte::beginRender();
    frostbyte::Frostbyte::endRender();
    frostbyte::Frostbyte::postRender();

    lua_gc(frostbyte::Frostbyte::L, LUA_GCCOLLECT, 64);
}

void tryRunCode(lua_State* L, const char* name, const char* code, size_t code_length, frostbyte::ScriptLanguage* language = nullptr, const frostbyte::ThreadIdentity* identity = nullptr) {
    try {
        lua_State* newthread = frostbyte::TaskScheduler::startCodeOnNewThread(L, name, code, code_length, language, identity);
        while (newthread && frostbyte::getTask(newthread))
            gameCycle();
    } catch(std::exception& e) {
        frostbyte::Console::ScriptConsole.error(e.what());
    }
}

std::atomic<bool> stop(false);

void handle_sigint(int) {
    stop = true;
    frostbyte::Frostbyte::cleanup(false);
}

int main(int argc, char** argv) {
    if (argc < 1) {
        displayHelp();
        return 1;
    }

    frostbyte::FrostbyteConfiguration configuration;

    frostbyte::ScriptLanguage* input_language = &frostbyte::ScriptLanguage::Luau;
    const char* input_file_path = nullptr;
    const char* input_code = nullptr;

    for (unsigned i = 1; i < (unsigned) argc; i++) {
        const char* arg = argv[i];
        if (strequal(arg, "-h") || strequal(arg, "--help")) {
            displayHelp();
            return 0;
        } else if (strequal(arg, "--nosandbox"))
            configuration.sandbox_enabled = false;
        else if (!handleRecordOption("--script", arg)) {
            if (input_code) {
                fprintf(stderr, "ERROR: you cannot pass both --script and --code\n");
                return 1;
            }
            input_file_path = arg;
        } else if (!handleRecordOption("--code", arg)) {
            if (input_file_path) {
                fprintf(stderr, "ERROR: you cannot pass both --script and --code\n");
                return 1;
            }
            input_code = arg;
        } else if (!handleRecordOption("--language", arg)) {
            if (strequal(arg, "luau"))
                input_language = &frostbyte::ScriptLanguage::Luau;
            else if (strequal(arg, "moonscript"))
                input_language = &frostbyte::ScriptLanguage::MoonScript;
            else if (strequal(arg, "clue"))
                input_language = &frostbyte::ScriptLanguage::Clue;
            else {
                fprintf(stderr, "ERROR: unsupported language '%s'; expected luau, moonscript, or clue\n", arg);
                return 1;
            }
        } else if (*arg) {
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

    frostbyte::Frostbyte::initialize(configuration);

    lua_State* L = frostbyte::Frostbyte::L;
    lua_State* appL = frostbyte::Frostbyte::appL;

    lua_State* userL = frostbyte::TaskScheduler::newThread(L, [] (std::string error) {
        if (!error.empty() && error.at(error.size() - 1) == '\n')
            error.erase(error.end() - 1);
        frostbyte::Console::ScriptConsole.error(error);
    });
    lua_pop(L, 1);
    frostbyte::Console::ScriptConsole.debugf("user state: %p", userL);

    {
        char buf[100];
        snprintf(buf, 100, "App State (%p)", appL);
        frostbyte::getTask(appL)->identifier.assign(buf);
        snprintf(buf, 100, "User State (%p)", userL);
        frostbyte::getTask(userL)->identifier.assign(buf);
    }

    gameCycle();

    struct sigaction action{};
    action.sa_handler = handle_sigint;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;

    sigaction(SIGINT, &action, nullptr);

    if (input_file_path) {
        std::string contents = readFileToString(input_file_path);
        tryRunCode(userL, input_file_path, contents.c_str(), contents.length(), input_language);
    } else if (input_code)
        tryRunCode(userL, "code", input_code, strlen(input_code), input_language);
    else {
        frostbyteserver::Interface::initialize();

        std::string input;
        input.reserve(50);

        while (!stop && !frostbyte::DataModel::shutdown && !frostbyteserver::Interface::wantsQuit()) {
            frostbyteserver::Interface::mainloop(input);
            if (input == "exit")
                break;

            tryRunCode(userL, "code", input.c_str(), input.size(), input_language);
        }
        frostbyteserver::Interface::cleanup();
    }

    if (!stop)
        frostbyte::Frostbyte::cleanup(false);

    return 0;
}
