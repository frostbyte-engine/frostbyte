#include "scriptlanguage.hpp"

#include "libraries/filesystemlib.hpp"

#include <cassert>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <stdexcept>
#include <sstream>
#include <vector>
#include <sys/wait.h>

namespace frostbyte {

ScriptLanguage ScriptLanguage::Luau(0, "Luau");
ScriptLanguage ScriptLanguage::MoonScript(1, "MoonScript");
ScriptLanguage ScriptLanguage::Clue(2, "Clue");

ScriptLanguage* ScriptLanguage::list[] = { &ScriptLanguage::Luau, &ScriptLanguage::MoonScript, &ScriptLanguage::Clue };
int ScriptLanguage::count = sizeof(ScriptLanguage::list) / sizeof(ScriptLanguage::list[0]);

inline bool doesFileExist(const char* name) {
    std::string path = FileSystem::bin_path;
    path.append(name);
    #ifdef __WIN32
    path.append(".exe");
    #endif

    return std::filesystem::exists(path);
}

void ScriptLanguage::refresh() {
    ScriptLanguage::Luau.enabled = true;

    ScriptLanguage::MoonScript.enabled = doesFileExist("moonc");
    ScriptLanguage::Clue.enabled = doesFileExist("clue");
}

void run_process(const char* executable, const std::initializer_list<std::string>& args, int& exit_code, std::string& stdout) {
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1)
        throw std::runtime_error(std::string("failed to convert language because pipe() failed: ") + strerror(errno));

    pid_t pid = fork();
    if (pid == -1) {
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        throw std::runtime_error(std::string("failed to convert language because fork() failed: ") + strerror(errno));
    }

    if (pid == 0) {
        close(pipe_fd[0]);
        dup2(pipe_fd[1], STDOUT_FILENO);
        dup2(pipe_fd[1], STDERR_FILENO);
        close(pipe_fd[1]);

        std::vector<const char*> argv;
        argv.push_back(executable);
        for (auto& a : args)
            argv.push_back(a.c_str());
        argv.push_back(nullptr);

        execvp(executable, const_cast<char* const*>(argv.data()));

        std::perror("execvp failed");
        _exit(127);
    }

    close(pipe_fd[1]);

    std::ostringstream oss;
    char buf[4096];
    ssize_t n;
    while ((n = read(pipe_fd[0], buf, sizeof(buf))) > 0)
        oss.write(buf, n);
    close(pipe_fd[0]);

    int status = 0;
    waitpid(pid, &status, 0);

    stdout.assign(oss.str());
    exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : -1;
}

void ScriptLanguage::convert(std::string& code) {
    if (this == &ScriptLanguage::Luau)
        return;

    assert(enabled);

    bool is_clue = this == &ScriptLanguage::Clue;

    static std::string temp_path;
    temp_path.assign(FileSystem::temp_path);
    temp_path.append("language_conversion.txt");
    // NOTE: Clue NEEDS the input file to end in .clue .........
    if (is_clue)
        temp_path.append(".clue");

    {
    std::ofstream file;

    file.open(temp_path, std::ios::out);

    if (!file.is_open())
        throw std::runtime_error("failed to convert language because failed to open temp file");

    file << code;
    file.close();
    }

    int exit_code;
    static std::string stdout;
    static std::string executable;
    executable.assign(FileSystem::bin_path);

    if (this == &ScriptLanguage::MoonScript) {
        executable.append("moonc");
        #ifdef _WIN32
        executable.append(".exe");
        #endif

        run_process(executable.c_str(), { "-p", temp_path.c_str() }, exit_code, stdout);

        if (exit_code)
            throw std::runtime_error(std::string("failed to compile moonscript:\n") + stdout);

        code.assign(stdout);
    } else if (is_clue) {
        executable.append("clue");
        #ifdef _WIN32
        executable.append(".exe");
        #endif

        run_process(executable.c_str(), { temp_path.c_str(), "-D", "-t", "Lua51", "-o" }, exit_code, stdout);

        int data_offset = 0;
        char* data = stdout.data();
        static std::string line;

        for (size_t i = 0; i < stdout.size(); i++) {
            char ch = stdout[i];

            if (ch == '\n') {
                if (line.rfind("Warning: ", 0) == 0) {
                    // TODO: it would be nice if we call ScriptConsole.warning here...
                    data_offset = i + 1;
                } else if (line.rfind("Compiled Lua code", 0) == 0)
                    data_offset = i + 1;
                line.clear();
            } else
                line.push_back(ch);
        }

        data += data_offset;

        if (exit_code)
            throw std::runtime_error(std::string("failed to compile clue:\n") + data);

        // minus one to remove trailing \n
        code.assign(data, stdout.size() - data_offset - 1);

        // remove last line
        auto pos = code.rfind('\n');
        if (pos != std::string::npos)
            code.erase(pos);
    }
}

}; // namespace frostbyte
