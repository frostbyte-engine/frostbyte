#define MATE_IMPLEMENTATION
#include "mate.h"

#define setpath(var, size, value, path) memset(var, 0, size); snprintf(var, size, "%s/%s", value, path);

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("usage: %s FROSTBYTE_PATH\n", argc ? argv[0] : "mate");
        return 1;
    }

    const char* frostbyte_path = argv[1];
    int path_buffer_size = strlen(frostbyte_path) + 40;
    char* path_buffer = (char*) malloc(path_buffer_size);

    StartBuild();

    ExecutableOptions executable_options = {
        .output = "frostbyte",
        .flags = "-std=c++17 -Wall -Wno-psabi -Werror -g -march=native -static-libstdc++ -static-libgcc"
    };
    Executable executable = CreateExecutable(executable_options);

    AddIncludePaths(executable, "./include");

    AddFile(executable, "./src/*.cpp");

    setpath(path_buffer, path_buffer_size, frostbyte_path, "include")
    AddIncludePaths(executable, path_buffer);
    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/json/include")
    AddIncludePaths(executable, path_buffer);
    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/curl/include")
    AddIncludePaths(executable, path_buffer);
    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/uuid_v4")
    AddIncludePaths(executable, path_buffer);
    AddIncludePaths(executable, "./dependencies/ImGuiFileDialog");

    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/luau/Analysis/include")
    AddIncludePaths(executable, path_buffer);
    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/luau/Ast/include")
    AddIncludePaths(executable, path_buffer);
    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/luau/Common/include")
    AddIncludePaths(executable, path_buffer);
    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/luau/Compiler/include")
    AddIncludePaths(executable, path_buffer);
    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/luau/Config/include")
    AddIncludePaths(executable, path_buffer);
    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/luau/VM/include")
    AddIncludePaths(executable, path_buffer);
    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/luau/VM/src")
    AddIncludePaths(executable, path_buffer);

    AddIncludePaths(executable, "./dependencies/rlImGui");
    AddIncludePaths(executable, "./dependencies/rlImGui/raylib-master/src");
    AddIncludePaths(executable, "./dependencies/rlImGui/imgui-master");

    setpath(path_buffer, path_buffer_size, frostbyte_path, "build")
    AddLibraryPaths(executable, path_buffer);
    LinkSystemLibraries(executable, "frostbyte");

    LinkSystemLibraries(executable, "m", "stdc++", "raylib", "X11");

    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/luau/cmake")
    AddLibraryPaths(executable, path_buffer);
    LinkSystemLibraries(executable, "Luau");

    setpath(path_buffer, path_buffer_size, frostbyte_path, "dependencies/curl/cmake/lib")
    AddLibraryPaths(executable, path_buffer);
    LinkSystemLibraries(executable, "curl");

    AddLibraryPaths(executable, "./dependencies/rlImGui/bin/Release");
    LinkSystemLibraries(executable, "rlImGui");

    AddLibraryPaths(executable, "./dependencies/ImGuiFileDialog/cmake");
    LinkSystemLibraries(executable, "ImGuiFileDialog");

    InstallExecutable(executable);

    EndBuild();

    free(path_buffer);
    return 0;
}
