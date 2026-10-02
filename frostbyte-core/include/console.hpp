#pragma once

#include <shared_mutex>
#include <string>
#include <vector>

#include "renderstubs.hpp"
#include "luaconf.h"

namespace frostbyte {

enum ConsoleId {
    Script,
    Tests
};

class Console {
    std::string whole_content;
public:
    static Console ScriptConsole;
    static Console TestsConsole;

    static Vector4 ColorINFO;
    static Vector4 ColorWARNING;
    static Vector4 ColorERROR;
    static Vector4 ColorDEBUG;

    class Message {
    public:
        enum Type {
            INFO,
            WARNING,
            ERROR,
            DEBUG
        } type;
        std::string content;
    };
    static const char* getMessageTypeString(Message::Type type);

    ConsoleId id;

    std::shared_mutex mutex;
    std::vector<Message> messages;

    bool show_info;
    bool show_warning;
    bool show_error;
    bool show_debug;

    Console(ConsoleId id, bool show_info = true, bool show_warning = true, bool show_error = true, bool show_debug = false);

    void clear();
    void renderMessages();
    // NOTE: not thread-safe
    std::string& getWholeContent();

    void log(std::string_view message, Message::Type type);

    void info(std::string_view message);
    void warning(std::string_view message);
    void error(std::string_view message);
    void debug(std::string_view message);

    void LUA_PRINTF_ATTR(3, 4) logf(Message::Type type, const char* fmt, ...);
    void LUA_PRINTF_ATTR(2, 3) debugf(const char* fmt, ...);
    void LUA_PRINTF_ATTR(2, 3) infof(const char* fmt, ...);
    void LUA_PRINTF_ATTR(2, 3) warningf(const char* fmt, ...);
    void LUA_PRINTF_ATTR(2, 3) errorf(const char* fmt, ...);
};

}; // namespace frostbyte
