#include "taskscheduler.hpp"

#include <algorithm>
#include <cassert>
#include <exception>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>

#include "raylib.h"

#include "common.hpp"

#include "Luau/Common.h"
#include "lua.h"
#include "luacode.h"
#include "lualib.h"
#include "lgc.h"

namespace frostbyte {

const ThreadCapability ThreadCapability::PLUGIN                ( 0,      0, "Plugin Security");
const ThreadCapability ThreadCapability::LOCAL_USER            ( 1, 1 << 0, "Local User Security");
const ThreadCapability ThreadCapability::WRITE_PLAYER          ( 2, 1 << 1, "Write Player Security");
const ThreadCapability ThreadCapability::ROBLOX_SCRIPT         ( 3, 1 << 2, "Roblox Script Security");
const ThreadCapability ThreadCapability::ROBLOX_ENGINE         ( 4, 1 << 3, "Roblox Security");
const ThreadCapability ThreadCapability::NOT_ACCESSIBLE        ( 5, 1 << 4, "Not Accessible Security");
const ThreadCapability ThreadCapability::REMOTE_COMMAND        (59, 1 << 5, "Remote Command Security");
const ThreadCapability ThreadCapability::INTERNAL_TEST         (60, 1 << 6, "Internal Test Security");
const ThreadCapability ThreadCapability::PLUGIN_OR_OPEN_CLOUD  (61, 1 << 7, "OpenCloud Security");
const ThreadCapability ThreadCapability::ASSISTANT             (62, 1 << 8, "Assistant Security");

const ThreadIdentity ThreadIdentity::ANONYMOUS              ( 0, 0                                                                                                                                                                                                                            , "Anonymous");
const ThreadIdentity ThreadIdentity::LOCAL_GUI              ( 1, ThreadCapability::PLUGIN.flag | ThreadCapability::LOCAL_USER.flag                                                                                                                                                            , "LocalGui");
const ThreadIdentity ThreadIdentity::GAME_SCRIPT            ( 2, ThreadCapability::REMOTE_COMMAND.flag                                                                                                                                                                                        , "GameScript");
const ThreadIdentity ThreadIdentity::ELEVATED_GAME_SCRIPT   ( 3, ThreadCapability::PLUGIN.flag | ThreadCapability::LOCAL_USER.flag | ThreadCapability::ROBLOX_SCRIPT.flag | ThreadCapability::INTERNAL_TEST.flag                                                                              , "ElevatedGameScript");
const ThreadIdentity ThreadIdentity::COMMAND_BAR            ( 4, ThreadCapability::PLUGIN.flag | ThreadCapability::LOCAL_USER.flag /*| ThreadCapability::INTERNAL_TEST.flag*/                                                                                                                 , "CommandBar");
const ThreadIdentity ThreadIdentity::STUDIO_PLUGIN          ( 5, ThreadCapability::PLUGIN.flag /*| ThreadCapability::INTERNAL_TEST.flag*/                                                                                                                                                     , "StudioPlugin");
const ThreadIdentity ThreadIdentity::ELEVATED_STUDIO_PLUGIN ( 6, ThreadCapability::PLUGIN.flag | ThreadCapability::LOCAL_USER.flag | ThreadCapability::ROBLOX_SCRIPT.flag | ThreadCapability::ASSISTANT.flag | ThreadCapability::INTERNAL_TEST.flag                                           , "ElevatedStudioPlugin");
const ThreadIdentity ThreadIdentity::COM                    ( 7, ThreadCapability::PLUGIN.flag | ThreadCapability::LOCAL_USER.flag | ThreadCapability::WRITE_PLAYER.flag | ThreadCapability::ROBLOX_SCRIPT.flag | ThreadCapability::ROBLOX_ENGINE.flag | ThreadCapability::NOT_ACCESSIBLE.flag, "COM");
const ThreadIdentity ThreadIdentity::WEB_SERVICE            ( 8, ThreadCapability::PLUGIN.flag | ThreadCapability::LOCAL_USER.flag | ThreadCapability::WRITE_PLAYER.flag | ThreadCapability::ROBLOX_SCRIPT.flag | ThreadCapability::ROBLOX_ENGINE.flag | ThreadCapability::NOT_ACCESSIBLE.flag, "WebService");
const ThreadIdentity ThreadIdentity::REPLICATOR             ( 9, ThreadCapability::WRITE_PLAYER.flag | ThreadCapability::ROBLOX_SCRIPT.flag                                                                                                                                                   , "Replicator");
const ThreadIdentity ThreadIdentity::ASSISTANT              (10, ThreadCapability::PLUGIN.flag | ThreadCapability::LOCAL_USER.flag | ThreadCapability::ASSISTANT.flag                                                                                                                         , "Assistant");
const ThreadIdentity ThreadIdentity::OPEN_CLOUD_SESSION     (11, ThreadCapability::PLUGIN_OR_OPEN_CLOUD.flag                                                                                                                                                                                  , "OpenCloudSession");
const ThreadIdentity ThreadIdentity::TESTING_GAME_SCRIPT    (12, ThreadCapability::INTERNAL_TEST.flag                                                                                                                                                                                         , "TestingGameScript");
const ThreadIdentity ThreadIdentity::UNDO_STACK             (13, ThreadCapability::WRITE_PLAYER.flag | ThreadCapability::ROBLOX_SCRIPT.flag                                                                                                                                                   , "UndoStack");

const char* taskStatusTostring(TaskStatus status) {
    switch (status) {
        case IDLE:
            return "idle";
        case RUNNING:
            return "running";
        case YIELDING:
            return "yielding";
        case WAITING:
            return "waiting";
        case DEFERRING:
            return "deferring";
        case DELAYING:
            return "delaying";
        default:
            return "unknown";
    }
};

bool TaskScheduler::sandboxing = true;
double TaskScheduler::initial_client_time = 0; // gets set in main

int TaskScheduler::target_fps = 240;
// std::shared_mutex TaskScheduler::target_fps_mutex;

void TaskScheduler::setTargetFps(int target) {
    // std::lock_guard lock(target_fps_mutex);

    target_fps = target;
    SetTargetFPS(target);
}

// std::shared_mutex TaskScheduler::gc_mutex;
bool TaskScheduler::gcShouldRun(lua_State* L) {
    // std::lock_guard lock(gc_mutex);

    return lua_gc(L, LUA_GCISRUNNING, 0);
}
bool TaskScheduler::gcActuallyPaused(lua_State* L) {
    return L->global->gcstate == GCSpause;
}
void TaskScheduler::gcCollect(lua_State* L) {
    // std::lock_guard lock(gc_mutex);

    lua_gc(L, LUA_GCCOLLECT, 0);
}
void TaskScheduler::pauseGarbageCollection(lua_State* L) {
    // std::lock_guard lock(gc_mutex);

    lua_gc(L, LUA_GCSTOP, 0);
}
void TaskScheduler::resumeGarbageCollection(lua_State* L) {
    // std::lock_guard lock(gc_mutex);

    lua_gc(L, LUA_GCRESTART, 0);
}

void TaskScheduler::performGCWork(lua_State* L, std::function<void()> work) {
    const bool resume_gc = !gcActuallyPaused(L);
    if (resume_gc)
        gcCollect(L);

    pauseGarbageCollection(L);

    work();

    if (resume_gc)
        resumeGarbageCollection(L);
}

lua_State* TaskScheduler::mainL = nullptr;

std::vector<lua_State*> TaskScheduler::thread_list;
// std::shared_mutex TaskScheduler::thread_list_mutex;

std::queue<Yield> TaskScheduler::pending_yield_list;
std::shared_mutex TaskScheduler::pending_yield_mutex;

std::vector<lua_State*> TaskScheduler::thread_queue;
// std::shared_mutex TaskScheduler::thread_queue_mutex;

void TaskScheduler::setup(lua_State *L) {
    mainL = L;

    lua_callbacks(L)->userthread = [] (lua_State* parent, lua_State* thread) {
        if (parent) {
            Task* task = new Task();
            task->status = IDLE;
            task->parent = parent;

            char buffer[32];
            std::snprintf(buffer, sizeof(buffer), "%p", static_cast<void*>(thread));
            task->identifier = std::string(buffer);

            auto parent_task = getTask(parent);
            if (parent_task)
                task->console = parent_task->console;

            task->canceled = false;
            task->arg_count = 0;

            lua_setthreaddata(thread, task);

            // std::lock_guard lock(TaskScheduler::thread_list_mutex);
            TaskScheduler::thread_list.push_back(thread);
        } else {
            killThread(thread);
            Task* task = getTask(thread);
            delete task;
            lua_setthreaddata(thread, nullptr);
        }
    };
}

lua_State* TaskScheduler::newThread(lua_State* L, Feedback feedback, OnKill on_kill) {
    lua_State* thread = lua_newthread(L);
    if (sandboxing)
        luaL_sandboxthread(thread);

    Task* task = getTask(thread);
    task->ref = lua_ref(L, -1);

    task->feedback = feedback;
    task->on_kill = on_kill;

    return thread;
}

void TaskScheduler::killThreadUnlocked(lua_State* thread) {
    Task* task = getTask(thread);
    lua_unref(task->parent, task->ref);

    // std::shared_lock thread_queue_lock(thread_queue_mutex);

    auto thread_queue_position = std::find(thread_queue.begin(), thread_queue.end(), thread);
    if (thread_queue_position != thread_queue.end())
        thread_queue.erase(thread_queue_position);

    // TODO: handle exception?
    if (task->on_kill)
        task->on_kill();
}
void TaskScheduler::killThread(lua_State* thread) {
    // std::shared_lock thread_list_lock(thread_list_mutex);

    auto thread_list_position = std::find(thread_list.begin(), thread_list.end(), thread);
    if (thread_list_position == thread_list.end())
        return;
    thread_list.erase(thread_list_position);

    // thread_list_lock.unlock();

    killThreadUnlocked(thread);
}

bool resumeThreadRaw(lua_State* thread) {
    Task* task = getTask(thread);

    task->status = RUNNING;

    int status = lua_resume(thread, task->parent, task->arg_count);

    switch (status) {
        case LUA_OK:
            return false;
        case LUA_YIELD:
            task->status = YIELDING;
            return true;
        case LUA_ERRRUN: {
            throw std::runtime_error(getErrorMessage(thread));
        }
        default: {
            std::string msg = "unexpected resume status: ";
            switch (status) {
                case LUA_ERRSYNTAX:
                    msg.append("ERRSYNTAX");
                    break;
                case LUA_ERRMEM:
                    msg.append("ERRMEM");
                    break;
                case LUA_ERRERR:
                    msg.append("ERRERR");
                    break;
                default:
                    break;
            }
            throw std::runtime_error(msg);
        }
    }
}

void tryResumeThreadRaw(lua_State* thread) {
    Task* task = getTask(thread);
    assert(task);

    if (task->canceled)
        return;

    try {
        const bool yield = resumeThreadRaw(thread);
        if (yield)
            return;
        TaskScheduler::killThread(thread);
    } catch (std::exception& e) {
        // TODO: either move killThread after and wrap feedback in another try catch or don't worry about the order (ask if feedback will care if thread was killed or not)
        TaskScheduler::killThread(thread);
        try {
            task->feedback(e.what());
        } catch (std::exception& e2) {
            printf("[err] failed to call feedback... here is original exception:\n%s", e.what());
            printf("[err] and here is why the feedback failed: %s\n", e2.what());
        }
    }
}

void TaskScheduler::startFunctionOnNewThread(lua_State* L, Feedback feedback, int arg_count, Console* console) {
    luaL_checktype(L, lua_gettop(L) - arg_count, LUA_TFUNCTION);

    lua_State* thread = newThread(L, feedback);
    lua_pop(L, 1);
    Task* task = getTask(thread);
    if (console) task->console = console;

    task->arg_count = arg_count;
    lua_xmove(L, thread, arg_count + 1);

    tryResumeThreadRaw(thread);
}

void TaskScheduler::startCodeOnNewThread(lua_State* L, const char* chunk_name, const char* code, size_t code_size, const ThreadIdentity* identity, Feedback feedback, OnKill on_kill, Console* console) {
    size_t bytecode_size = 0;
    char* bytecode = luau_compile(code, code_size, NULL, &bytecode_size);
    if (!bytecode)
        throw std::runtime_error("failed to allocate memory for script bytecode");

    lua_State* thread = newThread(L, feedback, on_kill);
    lua_pop(L, 1);

    int r = luau_load(thread, chunk_name, bytecode, bytecode_size, 0);
    free(bytecode);

    if (r) {
        std::string msg = std::string("failed to load chunk: ")
            .append(lua_tostring(thread, -1));
        lua_settop(thread, 0);

        killThread(thread);

        throw std::runtime_error(msg);
    }

    Task* task = getTask(thread);
    task->arg_count = 0;
    if (identity) task->identity = identity;
    if (console) task->console = console;

    tryResumeThreadRaw(thread);
}

int TaskScheduler::yieldThread(lua_State* thread) {
    Task* task = getTask(thread);
    assert(task);

    task->status = TaskStatus::YIELDING;
    return lua_yield(thread, 0);
}

int TaskScheduler::yieldForWork(lua_State* thread, YieldFunction callback, bool threaded) {
    Task* task = getTask(thread);
    assert(task);

    int r = yieldThread(thread);

    if (threaded) {
        std::thread t([thread, callback]() {
            Yield yield(thread, &pending_yield_list, &pending_yield_mutex);
            callback(yield);
        });

        t.detach();
    } else {
        Yield yield(thread, &pending_yield_list, &pending_yield_mutex);
        callback(yield);
    }

    return r;
}

void TaskScheduler::resumeThread(lua_State* thread) {
    // std::unique_lock thread_queue_lock(thread_queue_mutex);
    thread_queue.erase(std::find(thread_queue.begin(), thread_queue.end(), thread));
    // thread_queue_lock.unlock();

    Task* task = getTask(thread);

    if (task->canceled)
        return;

    if (task->timing.type == TaskTiming::Wait) {
        // NOTE: count becomes elapsed, so return it
        lua_pushnumber(thread, task->timing.count);
        task->arg_count = 1;
    }

    tryResumeThreadRaw(thread);
}

void TaskScheduler::run() {
    // std::shared_lock task_queue_lock(thread_queue_mutex);

    static std::vector<lua_State*> filtered_threads;
    filtered_threads.clear();

    for (size_t i = 0; i < thread_queue.size(); i++) {
        lua_State* thread = thread_queue[i];
        Task* task = getTask(thread);
        assert(task->status != RUNNING);

        bool passes = false;
        TaskTiming& timing = task->timing;

        switch (timing.type) {
            case TaskTiming::Instant:
                passes = true;
                break;
            case TaskTiming::Wait: {
                const double elapsed = lua_clock() - timing.start_time;
                passes = elapsed >= timing.count;
                if (passes) timing.count = elapsed;
                break;
            }
            case TaskTiming::Delay: {
                passes = (lua_clock() - timing.start_time) >= timing.count;
                break;
            }
        }

        if (passes)
            filtered_threads.push_back(thread);
    }
    // task_queue_lock.unlock();

    for (size_t i = 0; i < filtered_threads.size(); i++)
        resumeThread(filtered_threads[i]);
}

void TaskScheduler::cleanup() {
    mainL = nullptr;

    // std::lock_guard lock(thread_list_mutex);

    for (size_t i = 0; i < thread_list.size(); i++)
        killThreadUnlocked(thread_list[i]);

    thread_list.clear();
}

struct PreSpawnResult {
    lua_State* thread;
    int arg_count = 0;
};

std::optional<PreSpawnResult> preTaskSpawn(lua_State* L, const char* func_name, int arg_offset) {
    Task* parent_task = getTask(L);
    assert(parent_task);

    int arg1 = 1 + arg_offset;
    luaL_checkany(L, arg1);
    int arg1_type = lua_type(L, arg1);

    constexpr int msg_size = 42;
    char msg[msg_size];
    snprintf(msg, msg_size, "expected function or thread, got %s", lua_typename(L, arg1_type));
    luaL_argcheck(L, arg1_type == LUA_TFUNCTION || arg1_type == LUA_TTHREAD, arg1, msg);

    lua_State* thread;
    Task* task;

    int arg_count = lua_gettop(L) - arg_offset;

    switch (arg1_type) {
        case LUA_TTHREAD: {
            thread = lua_tothread(L, arg1);
            task = getTask(L);
            assert(task);

            const TaskStatus status = task->status;
            switch (status) {
                case RUNNING:
                case YIELDING:
                    break;
                case WAITING:
                case DEFERRING:
                case DELAYING:
                    luaL_error(L, "attempt to call %s on a thread that is %s", func_name, taskStatusTostring(status));
                    return std::nullopt;
                case IDLE:
                    LUAU_UNREACHABLE();
            }
            break;
        }
        case LUA_TFUNCTION: {
            thread = TaskScheduler::newThread(L, parent_task->feedback);
            task = getTask(thread);
            break;
        }
        default:
            LUAU_UNREACHABLE();
    }

    for (int i = 0; i < arg_count; i++) {
        lua_xpush(L, thread, arg1);
        lua_remove(L, arg1);
    }

    return PreSpawnResult{ .thread = thread, .arg_count = arg_count };
}

void TaskScheduler::queueThread(lua_State* thread) {
    // std::shared_lock lock(thread_queue_mutex);
    thread_queue.push_back(thread);
}

void TaskScheduler::queueForResume(lua_State* thread, int arg_count) {
    Task* task = getTask(thread);
    assert(task);

    task->arg_count = arg_count;
    task->timing = TaskTiming { .type = TaskTiming::Instant };
    TaskScheduler::queueThread(thread);
}

int fr_task_wait(lua_State* L) {
    Task* task = getTask(L);
    assert(task);

    const double seconds = getSeconds(L);

    task->status = WAITING;
    task->timing = TaskTiming{
        .type = TaskTiming::Wait,
        .start_time = lua_clock(),
        .count = seconds
    };
    task->arg_count = 0;
    TaskScheduler::queueThread(L);

    return lua_yield(L, 0);
}

int fr_task_spawn(lua_State* L) {
    auto result = *preTaskSpawn(L, "spawn", 0);

    int arg_count = result.arg_count;
    lua_State* thread = result.thread;
    Task* task = getTask(thread);
    assert(task);

    task->arg_count = arg_count - 1;

    tryResumeThreadRaw(thread);

    return 1;
}

int fr_task_defer(lua_State* L) {
    auto result = *preTaskSpawn(L, "defer", 0);

    int arg_count = result.arg_count;
    lua_State* thread = result.thread;
    Task* task = getTask(thread);
    assert(task);

    task->arg_count = arg_count - 1;
    task->status = DEFERRING;
    task->timing = TaskTiming{ .type = TaskTiming::Instant };

    TaskScheduler::queueThread(thread);

    return 1;
}

int fr_task_delay(lua_State* L) {
    auto result = *preTaskSpawn(L, "delay", 1);

    const double seconds = getSeconds(L);
    lua_remove(L, 1);

    int arg_count = result.arg_count;
    lua_State* thread = result.thread;
    Task* task = getTask(thread);
    assert(task);

    task->arg_count = arg_count - 1;
    task->status = DELAYING;
    task->timing = TaskTiming{
        .type = TaskTiming::Delay,
        .start_time = lua_clock(),
        .count = seconds
    };

    TaskScheduler::queueThread(thread);

    return 1;
}

int fr_task_cancel(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTHREAD);

    lua_State* thread = lua_tothread(L, 1);
    Task* task = getTask(thread);
    assert(task);

    task->canceled = true;
    return 0;
}

int fr_task_status(lua_State* L) {
    luaL_checktype(L, 1, LUA_TTHREAD);

    lua_State* thread = lua_tothread(L, 1);
    Task* task = getTask(thread);
    assert(task);

    lua_pushstring(L, taskStatusTostring(task->status));
    return 1;
}

void open_tasklib(lua_State *L) {
    lua_newtable(L);

    setfunctionfield(L, fr_task_spawn, "spawn", true);
    setfunctionfield(L, fr_task_defer, "defer", true);
    setfunctionfield(L, fr_task_delay, "delay", true);
    setfunctionfield(L, fr_task_cancel, "cancel", true);
    setfunctionfield(L, fr_task_status, "status", true);
    setfunctionfield(L, fr_task_wait, "wait", true);

    lua_setglobal(L, "task");
}

}; // namespace frostbyte
