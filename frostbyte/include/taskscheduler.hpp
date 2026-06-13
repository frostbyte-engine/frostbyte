#pragma once

#include <mutex>
#include <queue>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

#include "common.hpp"
#include "console.hpp"
#include "scriptlanguage.hpp"

#include "lua.h"

namespace frostbyte {

enum TaskStatus {
    IDLE,

    RUNNING,
    YIELDING,
    WAITING,
    DEFERRING,
    DELAYING
};

struct TaskTiming {
    enum {
        Instant,
        Wait,
        Delay
    } type = Instant;

    double start_time = 0.0;
    double count = 0.0;
};

// CREDITS: https://github.com/Pseudoreality/Roblox-Identities
// NOTE: I ported ^ over on June 4th, 2026 so our api json might not match thread capabilities/identities

class ThreadCapability {
public:
    uint8_t id = 0;
    uint16_t flag = 0;
    const char* name = "unknown capability";
private:
    ThreadCapability(uint8_t id, uint16_t flag, const char* name): id(id), flag(flag), name(name) {}

public:
    static const ThreadCapability PLUGIN;
    static const ThreadCapability INVALID;
    static const ThreadCapability LOCAL_USER;
    static const ThreadCapability WRITE_PLAYER;
    static const ThreadCapability ROBLOX_SCRIPT;
    static const ThreadCapability ROBLOX_ENGINE;
    static const ThreadCapability NOT_ACCESSIBLE;
    static const ThreadCapability REMOTE_COMMAND;
    static const ThreadCapability INTERNAL_TEST;
    static const ThreadCapability PLUGIN_OR_OPEN_CLOUD;
    static const ThreadCapability ASSISTANT;
};

class ThreadIdentity {
public:
    uint8_t id = 0;
    uint16_t capability = 0;
    const char* name = "unknown identity";
private:
    ThreadIdentity(uint8_t id, uint16_t capability, const char* name): id(id), capability(capability), name(name) {}

public:
    static const ThreadIdentity ANONYMOUS;
    static const ThreadIdentity LOCAL_GUI;
    static const ThreadIdentity GAME_SCRIPT;
    static const ThreadIdentity ELEVATED_GAME_SCRIPT;
    static const ThreadIdentity COMMAND_BAR;
    static const ThreadIdentity STUDIO_PLUGIN;
    static const ThreadIdentity ELEVATED_STUDIO_PLUGIN;
    static const ThreadIdentity COM;
    static const ThreadIdentity WEB_SERVICE;
    static const ThreadIdentity REPLICATOR;
    static const ThreadIdentity ASSISTANT;
    static const ThreadIdentity OPEN_CLOUD_SESSION;
    static const ThreadIdentity TESTING_GAME_SCRIPT;
    static const ThreadIdentity UNDO_STACK;
};

extern const std::unordered_map<uint8_t, const ThreadIdentity*> identity_map;

struct Task {
    TaskStatus status;
    lua_State* parent;

    Console* console = &Console::ScriptConsole;
    std::string identifier;

    bool canceled;
    int ref;
    int arg_count;
    TaskTiming timing;

    // TODO: maybe just always use console and remove feedback
    Feedback feedback;
    OnKill on_kill;

    const ThreadIdentity* identity = &ThreadIdentity::GAME_SCRIPT;

    struct {
        bool open = false;
    } view;
};

#define YIELD_KILL (-2)
#define YIELD_ERROR (-3)

class Yield;

using YieldFunction = std::function<void(Yield)>;
using YieldFinish = std::function<int(lua_State*)>;
class Yield {
    std::queue<Yield>* yield_list = nullptr;
    std::shared_mutex* yield_mutex = nullptr;
public:
    lua_State* state;
    YieldFinish finisher = nullptr;

    Yield(lua_State* state, std::queue<Yield>* yield_list, std::shared_mutex* yield_mutex) {
        this->state = state;
        this->yield_list = yield_list;
        this->yield_mutex = yield_mutex;
    }

    void finish(YieldFinish callback) {
        assert(!this->finisher);
        this->finisher = callback;
        std::lock_guard lock(*yield_mutex);
        yield_list->push(*this);
    }
};

class TaskScheduler {
    // static std::shared_mutex target_fps_mutex;

    // static std::shared_mutex gc_mutex;

    static std::vector<lua_State*> thread_queue;
    // static std::shared_mutex thread_queue_mutex;

    static void resumeThread(lua_State* thread);
    static void killThreadUnlocked(lua_State* thread);
public:
    static bool sandboxing;
    static double init_time;

    static void setup(lua_State* L);

    static lua_State* mainL;

    static std::vector<lua_State*> thread_list;
    // static std::shared_mutex thread_list_mutex;

    static std::queue<Yield> pending_yield_list;
    static std::shared_mutex pending_yield_mutex;

    static int target_fps;
    static void setTargetFps(int target);

    static bool gcShouldRun(lua_State* L);
    static bool gcActuallyPaused(lua_State* L);
    static void gcCollect(lua_State* L);
    static void pauseGarbageCollection(lua_State* L);
    static void resumeGarbageCollection(lua_State* L);

    static void performGCWork(lua_State* L, std::function<void()> work);

    static lua_State* newThread(lua_State* L, Feedback feedback, OnKill on_kill = nullptr);
    static void killThread(lua_State* thread);

    static void queueThread(lua_State* thread);
    static void queueForResume(lua_State* thread, int arg_count);

    // TODO: the api here is inconsistent (console instead of on_kill) since we only use this function in one place (tests)
    static void startFunctionOnNewThread(lua_State* L, Feedback feedback, int arg_count = 0, Console* console = nullptr);
    static void startCodeOnNewThread(lua_State* L, const char* chunk_name, const char* code, size_t code_size, ScriptLanguage* language, const ThreadIdentity* identity, Feedback feedback, OnKill on_kill = nullptr, Console* console = nullptr);

    static int yieldThread(lua_State* thread);
    static int yieldForWork(lua_State* thread, YieldFunction callback, bool threaded);

    static void run();

    static void cleanup();
};

Task* getTask(lua_State* thread);

const char* taskStatusTostring(TaskStatus status);

void open_tasklib(lua_State* L);

int fr_task_spawn(lua_State* L);

}; // namespace frostbyte
