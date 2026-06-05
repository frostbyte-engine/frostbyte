#include "libraries/filesystemlib.hpp"
#include "environment.hpp"

#include "lua.h"
#include "Luau/Compiler.h"
#include "lualib.h"
#include "taskscheduler.hpp"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <string_view>

namespace frostbyte {

std::string FileSystem::home_path;
std::string FileSystem::workspace_path;

static void checkPath(lua_State* L, const char* path) {
    if (!*path)
        luaL_error(L, "path cannot be empty");

    if (strstr(path, "/.") != NULL || strstr(path, "\\.") != NULL)
        luaL_error(L, "path contains invalid characters (slash followed by a dot)");
}

static int fr_appendfile(lua_State* L) {
    const char* relative_path = luaL_checkstring(L, 1);
    size_t datal;
    const char* data = luaL_checklstring(L, 2, &datal);

    checkPath(L, relative_path);

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);
    std::ofstream file;

    file.open(path, std::ios::app);

    if (!file.is_open())
        luaL_error(L, "failed to open file '%s'", relative_path);

    file << std::string(data, datal);
    file.close();

    return 0;
}
static int fr_appendfileasync(lua_State* L) {
    std::string relative_path = luaL_checkstring(L, 1);
    size_t datal;
    const char* dataraw = luaL_checklstring(L, 2, &datal);
    std::string data(dataraw, datal);

    checkPath(L, relative_path.c_str());

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);

    return TaskScheduler::yieldForWork(L, [path, relative_path, data] (Yield yield) {
        std::ofstream file;

        file.open(path, std::ios::app);

        if (!file.is_open())
            yield.finish([relative_path] (lua_State* L) {
                lua_pushfstring(L, "failed to open file '%s'", relative_path.c_str());
                return YIELD_ERROR;
            });

        file << data;
        file.close();

        yield.finish([](lua_State* L) {
            return 0;
        });
    }, true);
}
static int fr_readfile(lua_State* L) {
    const char* relative_path = luaL_checkstring(L, 1);

    checkPath(L, relative_path);

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);
    if (!std::filesystem::exists(path))
        luaL_error(L, "failed to open file '%s'", relative_path);

    std::ifstream file(path, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    lua_pushlstring(L, content.c_str(), content.size());
    return 1;
}
static int fr_readfileasync(lua_State* L) {
    std::string relative_path = luaL_checkstring(L, 1);

    checkPath(L, relative_path.c_str());

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);
    if (!std::filesystem::exists(path))
        luaL_error(L, "failed to open file '%s'", relative_path.c_str());

    return TaskScheduler::yieldForWork(L, [relative_path, path] (Yield yield) {
        std::ifstream file(path, std::ios::binary);
        std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        yield.finish([content] (lua_State* L) {
            lua_pushlstring(L, content.c_str(), content.size());
            return 1;
        });
    }, true);
}
static int fr_writefile(lua_State* L) {
    const char* relative_path = luaL_checkstring(L, 1);
    size_t datal;
    const char* data = luaL_checklstring(L, 2, &datal);

    checkPath(L, relative_path);

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);
    std::ofstream file;

    file.open(path, std::ios::out);

    if (!file.is_open())
        luaL_error(L, "failed to open file '%s'", relative_path);

    file << std::string(data, datal);
    file.close();

    return 0;
}
static int fr_writefileasync(lua_State* L) {
    std::string relative_path = luaL_checkstring(L, 1);
    size_t datal;
    const char* dataraw = luaL_checklstring(L, 2, &datal);
    std::string data(dataraw, datal);

    checkPath(L, relative_path.c_str());

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);

    return TaskScheduler::yieldForWork(L, [path, relative_path, data] (Yield yield) {
        std::ofstream file;

        file.open(path, std::ios::out);

        if (!file.is_open())
            yield.finish([relative_path] (lua_State* L) {
                lua_pushfstring(L, "failed to open file '%s'", relative_path.c_str());
                return YIELD_ERROR;
            });

        file << data;
        file.close();

        yield.finish([](lua_State* L) {
            return 0;
        });
    }, true);
}

static int fr_makefolder(lua_State* L) {
    const char* relative_path = luaL_checkstring(L, 1);

    checkPath(L, relative_path);

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);

    if (std::filesystem::is_directory(path))
        return 0;

    if (!std::filesystem::create_directory(path))
        luaL_error(L, "failed to create directory '%s'", relative_path);

    return 0;
}

static bool isFile(std::string_view path) {
    // TODO: symlinks too?
    const bool result = std::filesystem::is_regular_file(path);

    return result;
}
static int fr_isfile(lua_State* L) {
    const char* relative_path = luaL_checkstring(L, 1);

    checkPath(L, relative_path);

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);

    lua_pushboolean(L, isFile(path));
    return 1;
}
static int fr_isfolder(lua_State* L) {
    const char* relative_path = luaL_checkstring(L, 1);

    checkPath(L, relative_path);

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);

    lua_pushboolean(L, std::filesystem::is_directory(path));
    return 1;
}


static int fr_delfile(lua_State* L) {
    const char* relative_path = luaL_checkstring(L, 1);

    checkPath(L, relative_path);

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);

    if (!std::filesystem::exists(path))
        luaL_error(L, "no file or directory exists at path '%s'", relative_path);

    if (!isFile(path))
        luaL_error(L, "given path is not a file '%s'", relative_path);

    if (!std::filesystem::remove(path))
        luaL_error(L, "failed to delete file '%s'", relative_path);

    return 0;
}
static int fr_delfolder(lua_State* L) {
    const char* relative_path = luaL_checkstring(L, 1);

    checkPath(L, relative_path);

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);

    if (!std::filesystem::exists(path))
        luaL_error(L, "no file or directory exists at path '%s'", relative_path);

    if (!std::filesystem::is_directory(path))
        luaL_error(L, "given path is not a folder '%s'", relative_path);

    if (!std::filesystem::remove_all(path))
        luaL_error(L, "failed to delete folder '%s'", relative_path);

    return 0;
}

static int fr_dofile(lua_State* L) {
    const char* relative_path = luaL_checkstring(L, 1);

    checkPath(L, relative_path);

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);
    if (!std::filesystem::exists(path))
        luaL_error(L, "failed to open file '%s'", relative_path);

    std::ifstream file(path, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    std::string bytecode = Luau::compile(content);
    if (luau_load(L, relative_path, bytecode.data(), bytecode.size(), 0) != 0)
        luaL_error(L, "%s", lua_tostring(L, -1));

    int top_before = lua_gettop(L);
    lua_call(L, 0, -1);

    return lua_gettop(L) - top_before + 1;
}
static int fr_loadfile(lua_State* L) {
    const char* relative_path = luaL_checkstring(L, 1);

    checkPath(L, relative_path);

    std::string path = FileSystem::workspace_path;
    path.append(relative_path);
    if (!std::filesystem::exists(path))
        luaL_error(L, "failed to open file '%s'", relative_path);

    std::ifstream file(path, std::ios::binary);
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

    std::string bytecode = Luau::compile(content);
    if (luau_load(L, relative_path, bytecode.data(), bytecode.size(), 0) == 0)
        return 1;

    lua_pushnil(L);
    lua_insert(L, -2);
    return 2;
}

void open_filesystemlib(lua_State* L) {
    env_expose(appendfile)
    env_expose(appendfileasync)
    env_expose(readfile)
    env_expose(readfileasync)
    env_expose(writefile)
    env_expose(writefileasync)
    env_expose(makefolder)
    env_expose(isfile)
    env_expose(isfolder)
    env_expose(delfile)
    env_expose(delfolder)
    env_expose(dofile)
    env_expose(loadfile)
}

}; // namespace frostbyte
