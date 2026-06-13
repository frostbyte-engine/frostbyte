#define MATE_IMPLEMENTATION
#include "mate.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: %s RLIMGUI_PATH\n", argc ? argv[0] : "mate");
        return 1;
    }

    const char* rlimgui_path = argv[1];
    int path_buffer_size = strlen(rlimgui_path) + 20;
    char* path_buffer = (char*) malloc(path_buffer_size);
    memset(path_buffer, 0, path_buffer_size);

    StartBuild();

    StaticLibOptions lib_options = {
        .output = "libfrostbyte",
        .flags = "-std=c++17 -Wall -Werror -g -march=native -static-libstdc++ -static-libgcc -fPIC"
    };
    StaticLib lib = CreateStaticLib(lib_options);

    AddIncludePaths(lib, "./include", "./dependencies/json/include", "./dependencies/curl/include", "./dependencies/uuid_v4");

    AddFile(lib, "./src/*.cpp");
    AddFile(lib, "./src/libraries/*.cpp");
    AddFile(lib, "./src/ui/*.cpp");
    AddFile(lib, "./src/engine/classes/*.cpp");
    AddFile(lib, "./src/engine/classes/frostbyte/*.cpp");
    AddFile(lib, "./src/engine/datatypes/*.cpp");

    AddIncludePaths(lib, "./dependencies/luau/Analysis/include");
    AddIncludePaths(lib, "./dependencies/luau/Ast/include");
    AddIncludePaths(lib, "./dependencies/luau/Common/include");
    AddIncludePaths(lib, "./dependencies/luau/Compiler/include");
    AddIncludePaths(lib, "./dependencies/luau/Config/include");
    AddIncludePaths(lib, "./dependencies/luau/VM/include");
    AddIncludePaths(lib, "./dependencies/luau/VM/src");

    snprintf(path_buffer, path_buffer_size, "%s/raylib-master/src", rlimgui_path);
    AddIncludePaths(lib, path_buffer);
    memset(path_buffer, 0, path_buffer_size);
    snprintf(path_buffer, path_buffer_size, "%s/imgui-master", rlimgui_path);
    AddIncludePaths(lib, path_buffer);

    // AddLibraryPaths(lib, "./dependencies/luau/cmake");

    memset(path_buffer, 0, path_buffer_size);
    snprintf(path_buffer, path_buffer_size, "%s/bin/Release", rlimgui_path);
    AddLibraryPaths(lib, path_buffer);

    // AddLibraryPaths(lib, "./dependencies/curl/cmake/lib");

    InstallStaticLib(lib);

    EndBuild();

    free(path_buffer);
    return 0;
}
