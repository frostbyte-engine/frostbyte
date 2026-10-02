#pragma once

#include <string>
namespace frostbyte {

class ScriptLanguage {
public:
    int id;
    const char* name;
    bool enabled = true;
private:
    ScriptLanguage(int id, const char* name): id(id), name(name) {}
public:
    static ScriptLanguage Luau;
    static ScriptLanguage MoonScript;
    static ScriptLanguage Clue;

    static int count;
    static ScriptLanguage* list[];

    static void refresh();

    void convert(std::string& code);
};

}; // namespace frostbyte
