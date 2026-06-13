#define MATE_IMPLEMENTATION
#include "mate.h"

int main() {
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

    // AddIncludePaths(lib, "../rlImGui");
    AddIncludePaths(lib, "../rlImGui/raylib-master/src");
    AddIncludePaths(lib, "../rlImGui/imgui-master");

    // LinkSystemLibraries(lib, "m", "stdc++", "raylib", "X11");

    AddLibraryPaths(lib, "./dependencies/luau/cmake");
    // LinkSystemLibraries(lib, "Luau");

    AddLibraryPaths(lib, "../rlImGui/bin/Release");
    // LinkSystemLibraries(lib, "rlImGui");

    AddLibraryPaths(lib, "./dependencies/curl/cmake/lib");
    // LinkSystemLibraries(lib, "curl");

    // AddLibraryPaths(lib, "./dependencies/ImGuiFileDialog/cmake");
    // LinkSystemLibraries(lib, "ImGuiFileDialog");

    InstallStaticLib(lib);

    EndBuild();
    return 0;
}
