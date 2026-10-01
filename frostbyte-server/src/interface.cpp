#include "interface.hpp"
#include "common.hpp"
#include "sysutils.hpp"

namespace frostbyteserver {

int Interface::rows, Interface::cols;
std::vector<std::string> Interface::lines, Interface::history;
std::string Interface::line;
size_t Interface::pos = 0, Interface::history_index = 0;
bool Interface::wants_quit = false;

WINDOW* Interface::status = nullptr;
WINDOW* Interface::body = nullptr;

std::string prompt = "Enter code (or type exit): ";

void Interface::emit(const std::string& text) {
    size_t start = 0;
    for (;;) {
        size_t nl = text.find('\n', start);
        lines.push_back(text.substr(start, nl == std::string::npos ? nl : nl - start));
        if (nl == std::string::npos) break;
        start = nl + 1;
    }
}
void Interface::initialize() {
    frostbyte::print_func = [] (char* message) {
        Interface::emit(message);
    };

    initscr();
    raw();
    noecho();

    getmaxyx(stdscr, rows, cols);
    status = newwin(1, cols, 0, 0);
    body = newwin(rows - 1, cols, 1, 0);
    keypad(body, TRUE);
    wtimeout(body, 250);
}

void Interface::drawStatus() {
    werase(status);
    wattron(status, A_REVERSE);
    int cols = getmaxx(status);
    wmove(status, 0, 0);
    for (int i = 0; i < cols; i++)
        waddch(status, ' ');
    mvwprintw(status, 0, 1, "frostbyte-server    |    RAM Usage: %.2f GB / %.2f GB (%f%%)", frostbyte::SysUtils::physical_memory_used, frostbyte::SysUtils::physical_memory_total, frostbyte::SysUtils::physical_memory_used / frostbyte::SysUtils::physical_memory_total * 100.0);
    wattroff(status, A_REVERSE);
    wnoutrefresh(status);
}
void Interface::drawBody() {
    int h, cols;
    getmaxyx(body, h, cols);
    werase(body);

    auto getRows = [&cols] (size_t n) {
        return (int)std::max<size_t>(1, (n + cols - 1) / cols);
    };

    int plen = static_cast<int>(prompt.size());
    int in_rows = (int)(plen + line.size()) / cols + 1;
    int budget = h - in_rows;

    size_t first = lines.size();
    while (first > 0) {
        int r = getRows(lines[first - 1].size());
        if (r > budget)
            break;
        budget -= r;
        --first;
    }

    int y = 0;
    for (size_t i = first; i < lines.size(); i++) {
        mvwaddstr(body, y, 0, lines[i].c_str());
        y += getRows(lines[i].size());
    }
    mvwaddstr(body, y, 0, prompt.c_str());
    waddstr(body, line.c_str());
    int cur = plen + (int)pos;
    wmove(body, y + cur / cols, cur % cols);
    wnoutrefresh(body); // refresh last so the cursor lands here
}

void Interface::mainloop(std::string& code) {
    code.clear();

    drawStatus();
    drawBody();
    doupdate();

    int ch = wgetch(body);
    switch (ch) {
        case ERR:
            break;
        // Ctrl-C / Ctrl-D
        case 3: case 4:
            wants_quit = true;
            break;
        case KEY_RESIZE:
            getmaxyx(stdscr, rows, cols);
            wresize(status, 1, cols);
            wresize(body, rows - 1, cols);
            mvwin(body, 1, 0);
            clearok(stdscr, TRUE);
            break;
        case '\n': case KEY_ENTER:
            lines.push_back(prompt + line);
            code = line;

            if (!line.empty()) history.push_back(line);
            history_index = history.size();
            line.clear(); pos = 0;
            break;
        case KEY_BACKSPACE:
        case 127:
        case '\b':
            if (pos > 0)
                line.erase(--pos, 1);
            break;
        case KEY_DC:
            if (pos < line.size())
                line.erase(pos, 1);
            break;
        case KEY_LEFT:
            if (pos > 0)
                pos--;
            break;
        case KEY_RIGHT:
            if (pos < line.size())
                pos++;
            break;
        case KEY_HOME:
            pos = 0;
            break;
        case KEY_END:
            pos = line.size();
            break;
        case KEY_UP:
            if (history_index > 0) {
                line = history[--history_index];
                pos = line.size();
            }
            break;
        case KEY_DOWN:
            if (history_index + 1 < history.size()) {
                line = history[++history_index];
                pos = line.size();
            } else {
                history_index = history.size();
                line.clear();
                pos = 0;
            }
            break;
        default:
            if (ch >= 32 && ch < 127)
                line.insert(pos++, 1, static_cast<char>(ch));
    }
}
// print lines + last input on cleanup
void Interface::cleanup() {
    endwin();
    for (auto& l : lines)
        puts(l.c_str());
    if (!line.empty())
        printf("%s%s\n", prompt.c_str(), line.c_str());
}

bool Interface::wantsQuit() {
    return wants_quit;
}

}
