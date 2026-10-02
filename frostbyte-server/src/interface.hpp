#pragma once

#ifdef _WIN32
    // we use pdcurses on Windows
    #include <curses.h>
#else
    #include <ncurses.h>
#endif
#undef getstr

#include <string>
#include <vector>
namespace frostbyteserver {

class Interface {
    static int rows, cols;
    static std::vector<std::string> lines, history;
    static std::string line;
    static size_t pos, history_index;
    static bool wants_quit;

    static WINDOW* status;
    static WINDOW* body;

    static void drawStatus();
    static void drawBody();
public:
    static void emit(const std::string& text);
    static void initialize();
    static void mainloop(std::string& code);
    static void cleanup();

    static bool wantsQuit();
};

}
