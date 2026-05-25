#include "engine/classes/userinputservice.hpp"
#include "engine/classes/baseplayergui.hpp"
#include "engine/classes/instance.hpp"
#include "engine/classes/player.hpp"
#include "engine/classes/serviceprovider.hpp"
#include "engine/datatypes/vector2.hpp"
#include "engine/datatypes/enum.hpp"
#include "engine/datatypes/rbxscriptsignal.hpp"

#include "console.hpp"
#include "taskscheduler.hpp"

#include "raylib.h"

#include "lualib.h"

#include <map>
#include <queue>
#include <stdexcept>

namespace frostbyte {

void genericFire(lua_State* L, std::shared_ptr<rbxInstance> instance, const char* event, std::function<int(void)> pushArgs) {
    pushFunctionFromLookup(L, fireRBXScriptSignal);
    instance->pushEvent(L, event);

    lua_call(L, 1 + pushArgs(), 0);
}

void genericFire(lua_State* L, std::shared_ptr<rbxInstance> instance, const char* event) {
    return genericFire(L, instance, event, []{ return 0; });
}

void genericFireInputObject(lua_State* L, std::shared_ptr<rbxInstance> instance, const char* event, std::shared_ptr<rbxInstance> input_object, bool game_processed) {
    pushFunctionFromLookup(L, fireRBXScriptSignal);
    instance->pushEvent(L, event);

    lua_pushinstance(L, input_object);
    lua_pushboolean(L, game_processed);

    lua_call(L, 3, 0);
}

#define KEYCODE_MAP                             \
    X(KEY_NULL, "Unknown", NULL)                \
                                                \
    X(KEY_APOSTROPHE, "Quote", '\'')            \
    X(KEY_COMMA, "Comma", ',')                  \
    X(KEY_MINUS, "Minus", '-')                  \
    X(KEY_PERIOD, "Period", '.')                \
    X(KEY_SLASH, "Slash", '/')                  \
    X(KEY_ZERO, "Zero", '0')                    \
    X(KEY_ONE, "One", '1')                      \
    X(KEY_TWO, "Two", '2')                      \
    X(KEY_THREE, "Three", '3')                  \
    X(KEY_FOUR, "Four", '4')                    \
    X(KEY_FIVE, "Five", '5')                    \
    X(KEY_SIX, "Six", '6')                      \
    X(KEY_SEVEN, "Seven", '7')                  \
    X(KEY_EIGHT, "Eight", '8')                  \
    X(KEY_NINE, "Nine", '9')                    \
    X(KEY_SEMICOLON, "Semicolon", ';')          \
    X(KEY_EQUAL, "Equals", '=')                 \
                                                \
    X(KEY_A, "A", 'a')                          \
    X(KEY_B, "B", 'b')                          \
    X(KEY_C, "C", 'c')                          \
    X(KEY_D, "D", 'd')                          \
    X(KEY_E, "E", 'e')                          \
    X(KEY_F, "F", 'f')                          \
    X(KEY_G, "G", 'g')                          \
    X(KEY_H, "H", 'h')                          \
    X(KEY_I, "I", 'i')                          \
    X(KEY_J, "J", 'j')                          \
    X(KEY_K, "K", 'k')                          \
    X(KEY_L, "L", 'l')                          \
    X(KEY_M, "M", 'm')                          \
    X(KEY_N, "N", 'n')                          \
    X(KEY_O, "O", 'o')                          \
    X(KEY_P, "P", 'p')                          \
    X(KEY_Q, "Q", 'q')                          \
    X(KEY_R, "R", 'r')                          \
    X(KEY_S, "S", 's')                          \
    X(KEY_T, "T", 't')                          \
    X(KEY_U, "U", 'u')                          \
    X(KEY_V, "V", 'v')                          \
    X(KEY_W, "W", 'w')                          \
    X(KEY_X, "X", 'x')                          \
    X(KEY_Y, "Y", 'y')                          \
    X(KEY_Z, "Z", 'z')                          \
                                                \
    X(KEY_LEFT_BRACKET, "LeftBracket", '[')     \
    X(KEY_BACKSLASH, "BackSlash", '\\')         \
    X(KEY_RIGHT_BRACKET, "RightBracket", ']')   \
    X(KEY_GRAVE, "Backquote", '`')              \
                                                \
    X(KEY_SPACE, "Space", ' ')                  \
    X(KEY_ESCAPE, "Escape", NULL)               \
    X(KEY_ENTER, "Return", NULL)                \
    X(KEY_TAB, "Tab", ' ')                      \
    X(KEY_BACKSPACE, "Backspace", NULL)         \
    X(KEY_INSERT, "Insert", NULL)               \
    X(KEY_DELETE, "Delete", NULL)               \
    X(KEY_RIGHT, "Right", NULL)                 \
    X(KEY_LEFT, "Left", NULL)                   \
    X(KEY_DOWN, "Down", NULL)                   \
    X(KEY_UP, "Up", NULL)                       \
    X(KEY_PAGE_UP, "PageUp", NULL)              \
    X(KEY_PAGE_DOWN, "PageDown", NULL)          \
    X(KEY_HOME, "Home", NULL)                   \
    X(KEY_END, "End", NULL)                     \
    X(KEY_CAPS_LOCK, "CapsLock", NULL)          \
    X(KEY_SCROLL_LOCK, "ScrollLock", NULL)      \
    X(KEY_NUM_LOCK, "NumLock", NULL)            \
    X(KEY_PRINT_SCREEN, "Print", NULL)          \
    X(KEY_PAUSE, "Pause", NULL)                 \
    X(KEY_F1, "F1", NULL)                       \
    X(KEY_F2, "F2", NULL)                       \
    X(KEY_F3, "F3", NULL)                       \
    X(KEY_F4, "F4", NULL)                       \
    X(KEY_F5, "F5", NULL)                       \
    X(KEY_F6, "F6", NULL)                       \
    X(KEY_F7, "F7", NULL)                       \
    X(KEY_F8, "F8", NULL)                       \
    X(KEY_F9, "F9", NULL)                       \
    X(KEY_F10, "F10", NULL)                     \
    X(KEY_F11, "F11", NULL)                     \
    X(KEY_F12, "F12", NULL)                     \
    X(KEY_LEFT_SHIFT, "LeftShift", NULL)        \
    X(KEY_LEFT_CONTROL, "LeftControl", NULL)    \
    X(KEY_LEFT_ALT, "LeftAlt", NULL)            \
    X(KEY_LEFT_SUPER, "LeftSuper", NULL)        \
    X(KEY_RIGHT_SHIFT, "RightShift", NULL)      \
    X(KEY_RIGHT_CONTROL, "RightControl", NULL)  \
    X(KEY_RIGHT_ALT, "RightAlt", NULL)          \
    X(KEY_RIGHT_SUPER, "RightSuper", NULL)      \
    X(KEY_KB_MENU, "Menu", NULL)                \
                                                \
    X(KEY_KP_0, "KeypadZero", '0')              \
    X(KEY_KP_1, "KeypadOne", '1')               \
    X(KEY_KP_2, "KeypadTwo", '2')               \
    X(KEY_KP_3, "KeypadThree", '3')             \
    X(KEY_KP_4, "KeypadFour", '4')              \
    X(KEY_KP_5, "KeypadFive", '5')              \
    X(KEY_KP_6, "KeypadSix", '6')               \
    X(KEY_KP_7, "KeypadSeven", '7')             \
    X(KEY_KP_8, "KeypadEight", '8')             \
    X(KEY_KP_9, "KeypadNine", '9')              \
    X(KEY_KP_DECIMAL, "KeypadPeriod", '.')      \
    X(KEY_KP_DIVIDE, "KeypadDivide", '/')       \
    X(KEY_KP_MULTIPLY, "KeypadMultiply", '*')   \
    X(KEY_KP_SUBTRACT, "KeypadMinus", '-')      \
    X(KEY_KP_ADD, "KeypadPlus", '+')            \
    X(KEY_KP_ENTER, "KeypadEnter", NULL)        \
    X(KEY_KP_EQUAL, "KeypadEquals", '=')

static std::unordered_map<unsigned int, const char*> raylib_key_to_keycode_map = {
#define X(key, value, _) { key, value},
KEYCODE_MAP
#undef X
};
static std::unordered_map<unsigned int, char> raylib_key_to_char_map = {
#define X(key, _, value) { key, value},
KEYCODE_MAP
#undef X
};
#undef KEYCODE_MAP

#define KEYCODE_SHIFTED_MAP                  \
    X(KEY_APOSTROPHE, "QuotedDouble", '"')   \
    X(KEY_COMMA, "LessThan", '<')            \
    X(KEY_MINUS, "Underscore", '_')          \
    X(KEY_PERIOD, "GreaterThan", '>')        \
    X(KEY_SLASH, "Question", '?')            \
    X(KEY_ZERO, "RightParenthesis", ')')     \
    X(KEY_ONE, "One", '!')                   \
    X(KEY_TWO, "At", '@')                    \
    X(KEY_THREE, "Hash", '#')                \
    X(KEY_FOUR, "Dollar", '$')               \
    X(KEY_FIVE, "Percent", '%')              \
    X(KEY_SIX, "Caret", '^')                 \
    X(KEY_SEVEN, "Ampersand", '&')           \
    X(KEY_EIGHT, "Asterisk", '*')            \
    X(KEY_NINE, "LeftParenthesis", '(')      \
    X(KEY_SEMICOLON, "Colon", ':')           \
    X(KEY_EQUAL, "Plus", '+')                \
    X(KEY_A, "A", 'A')                       \
    X(KEY_B, "B", 'B')                       \
    X(KEY_C, "C", 'C')                       \
    X(KEY_D, "D", 'D')                       \
    X(KEY_E, "E", 'E')                       \
    X(KEY_F, "F", 'F')                       \
    X(KEY_G, "G", 'G')                       \
    X(KEY_H, "H", 'H')                       \
    X(KEY_I, "I", 'I')                       \
    X(KEY_J, "J", 'J')                       \
    X(KEY_K, "K", 'K')                       \
    X(KEY_L, "L", 'L')                       \
    X(KEY_M, "M", 'M')                       \
    X(KEY_N, "N", 'N')                       \
    X(KEY_O, "O", 'O')                       \
    X(KEY_P, "P", 'P')                       \
    X(KEY_Q, "Q", 'Q')                       \
    X(KEY_R, "R", 'R')                       \
    X(KEY_S, "S", 'S')                       \
    X(KEY_T, "T", 'T')                       \
    X(KEY_U, "U", 'U')                       \
    X(KEY_V, "V", 'V')                       \
    X(KEY_W, "W", 'W')                       \
    X(KEY_X, "X", 'X')                       \
    X(KEY_Y, "Y", 'Y')                       \
    X(KEY_Z, "Z", 'Z')                       \
    X(KEY_LEFT_BRACKET, "LeftCurly", '{')    \
    X(KEY_BACKSLASH, "Pipe", '|')            \
    X(KEY_RIGHT_BRACKET, "RightCurly", '}')  \
    X(KEY_GRAVE, "Tilde", '~')

static std::unordered_map<unsigned int, const char*> raylib_key_to_keycode_shifted_map = {
#define X(key, value, _) { key, value},
KEYCODE_SHIFTED_MAP
#undef X
};
static std::unordered_map<unsigned int, char> raylib_key_to_char_shifted_map = {
#define X(key, _, value) { key, value},
KEYCODE_SHIFTED_MAP
#undef X
};

#undef KEYCODE_SHIFTED_MAP

const int MAX_KEY = KEY_KB_MENU;

std::array<std::shared_ptr<rbxInstance>, MAX_KEY + 6> input_object_array; 

// const char* getKeyCodeName(lua_State* L, bool shift, unsigned int key) {
//     std::unordered_map<unsigned int, const char*>* map = &raylib_key_to_keycode_map;

//     if (shift)
//         map = &raylib_key_to_keycode_shifted_map;

//     auto it = map->find(key);
//     if (it == map->end())
//         return nullptr;

//     return it->second;
// }
char getKeyCodeCharacter(lua_State* L, bool shift, unsigned int key) {
    std::unordered_map<unsigned int, char>* map = &raylib_key_to_char_map;

    if (shift)
        map = &raylib_key_to_char_shifted_map;

    auto it = map->find(key);
    if (it == map->end())
        return '\0';

    return it->second;
}

const char* MOUSE_BUTTON_MAP[] = {
    "MouseButton1",
    "MouseButton2",
    "MouseButton3"
};

const char* INPUT_SIGNAL_MAP[] = {
    "InputBegan",
    "InputChanged",
    "InputEnded"
};
const char* INPUT_STATE_MAP[] = {
    "Begin",
    "Change",
    "End"
};

struct InputEventMouseClick {
    unsigned int mouse;
};
struct InputEventKeyboard {
    unsigned int key;
    const char* keycode;
};
struct InputEvent {
    enum {
        MouseClick,
        MouseMovement,
        MouseWheel,
        Keyboard
    } type;

    InputState state;
    std::weak_ptr<rbxInstance> instance;

    union {
        InputEventMouseClick mouse_click;
        InputEventKeyboard keyboard;
    };
};

std::queue<InputEvent> input_event_queue;
std::mutex input_event_mutex;

void pushInputEvent(InputEvent& event) {
    std::lock_guard<std::mutex> lock(input_event_mutex);

    input_event_queue.push(event);
}

void UserInputService::signalMouseMovement(std::shared_ptr<rbxInstance> instance, InputState type) {
    InputEvent event = {
        .type = InputEvent::MouseMovement,
        .state = type,
        .instance = instance
    };
    pushInputEvent(event);
}

int global_mouse_wheel = 0;
bool UserInputService::is_window_focused = false;
Vector2 UserInputService::mouse_position = GetMousePosition();
Vector2 UserInputService::mouse_delta{ 0.f, 0.f };
std::weak_ptr<rbxInstance> last_input;
std::weak_ptr<rbxInstance> last_topmost;
std::weak_ptr<rbxInstance> focused_textbox;

void UserInputService::releaseTextBoxFocus(lua_State* L, std::shared_ptr<rbxInstance> textbox, bool enter_pressed) {
    genericFire(L, ServiceProvider::service_map.at("UserInputService"), "TextBoxFocusReleased", [&L, textbox] {
        lua_pushinstance(L, textbox);
        return 1;
    });
    genericFire(L, textbox, "FocusLost", [&L, &enter_pressed] {
        lua_pushboolean(L, enter_pressed);
        return 1;
    });
}
void UserInputService::captureTextBoxFocus(lua_State* L, std::shared_ptr<rbxInstance> textbox) {
    auto previous_focused = focused_textbox.lock();
    focused_textbox = textbox;
    if (previous_focused == textbox)
        return;

    if (previous_focused)
        releaseTextBoxFocus(L, previous_focused, false);

    genericFire(L, ServiceProvider::service_map.at("UserInputService"), "TextBoxFocused", [&L, textbox] {
        lua_pushinstance(L, textbox);
        return 1;
    });
    genericFire(L, textbox, "Focused");
}
bool UserInputService::isTextBoxFocused(std::shared_ptr<rbxInstance> textbox) {
    if (auto focused = focused_textbox.lock())
        return textbox == focused;
    return false;
}

void UserInputService::process(lua_State *L, bool anyImGui) {
    UserInputService::mouse_delta = GetMouseDelta();
    UserInputService::mouse_position = GetMousePosition();
    const Vector2 mouse_wheel_vector = GetMouseWheelMoveV();

    const int mouse_wheel_y = mouse_wheel_vector.y;
    const int mouse_wheel = mouse_wheel_y ? (mouse_wheel_y > 0 ? 1 : -1) : 0;

    const bool is_window_focused = IsWindowFocused();
    if (is_window_focused != UserInputService::is_window_focused) {
        signalMouseMovement(nullptr, is_window_focused ? InputBegan : InputEnded);

        genericFire(L, ServiceProvider::service_map.at("UserInputService"), is_window_focused ? "WindowFocused" : "WindowFocusReleased");
    }
    UserInputService::is_window_focused = is_window_focused;

    for (unsigned int mouse = 0; mouse < 3; mouse++) {
        const bool pressed = IsMouseButtonPressed(mouse);
        const bool released = IsMouseButtonReleased(mouse);

        if (pressed || released) {
            InputEvent event = {
                .type = InputEvent::MouseClick,
                .state = pressed ? InputBegan : InputEnded,
                .mouse_click = {
                    .mouse = mouse
                }
            };
            pushInputEvent(event);
        }
    }

    if (mouse_delta.x || mouse_delta.y) {
        InputEvent event = {
            .type = InputEvent::MouseMovement,
            .state = InputChanged
        };
        pushInputEvent(event);
    }

    if (mouse_wheel && mouse_wheel != global_mouse_wheel) {
        global_mouse_wheel = mouse_wheel;
        InputEvent event = {
            .type = InputEvent::MouseWheel,
            .state = InputChanged
        };
        pushInputEvent(event);
    }

    // TODO: cache this?
    const bool shift = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT);

    {
    auto textbox = focused_textbox.lock();
    for (const auto& keycodepair : raylib_key_to_keycode_map) {
        unsigned int key = keycodepair.first;

        const bool pressed = IsKeyPressed(key);
        const bool released = IsKeyReleased(key);

        const char* keycode = keycodepair.second;

        if (textbox && pressed) {
            char ch = getKeyCodeCharacter(L, shift, key);
            auto& string = textbox->getValue<std::string>("Text");
            if (ch) {
                int pos = getInstanceValue<int>(textbox, "CursorPosition")++;
                string.insert(string.begin() + pos - 1, ch);
            } else if (strequal(keycode, "Return") || strequal(keycode, "KeypadEnter")) {
                focused_textbox.reset();
                releaseTextBoxFocus(L, textbox, true);
            } else if (strequal(keycode, "Escape")) {
                focused_textbox.reset();
                releaseTextBoxFocus(L, textbox, false);
            } else if (strequal(keycode, "Backspace") && string.size()) {
                int& pos = getInstanceValue<int>(textbox, "CursorPosition");
                if (pos > 1) {
                    pos--;
                    string.erase(string.begin() + pos - 1);
                }
            } else if (strequal(keycode, "Delete") && string.size()) {
                int& pos = getInstanceValue<int>(textbox, "CursorPosition");
                if (pos <= static_cast<int>(string.size()))
                    string.erase(string.begin() + pos - 1);
            } else if (strequal(keycode, "Home"))
                getInstanceValue<int>(textbox, "CursorPosition") = 1;
            else if (strequal(keycode, "End"))
                getInstanceValue<int>(textbox, "CursorPosition") = string.size() + 1;
            else if (strequal(keycode, "Left")) {
                int& pos = getInstanceValue<int>(textbox, "CursorPosition");
                if (pos > 1)
                    pos--;
            } else if (strequal(keycode, "Right")) {
                int& pos = getInstanceValue<int>(textbox, "CursorPosition");
                if (pos <= static_cast<int>(string.size()))
                    pos++;
            }

            continue;
        }

        if (pressed || released) {
            InputEvent event = {
                .type = InputEvent::Keyboard,
                .state = pressed ? InputBegan : InputEnded,
                .keyboard = {
                    .key = key,
                    .keycode = keycode,
                }
            };
            pushInputEvent(event);
        }
    }
    }

    auto topmost = getTopMostGuiObject().lock();
    while (!input_event_queue.empty()) {
        auto& event = input_event_queue.front();

        size_t array_index;
        switch (event.type) {
            case InputEvent::MouseClick:
                array_index = MAX_KEY + 1 + event.mouse_click.mouse;
                break;
            case InputEvent::MouseMovement:
                array_index = MAX_KEY + 4;
                break;
            case InputEvent::MouseWheel:
                array_index = MAX_KEY + 5;
                break;
            case InputEvent::Keyboard:
                array_index = event.keyboard.key;
                break;
        }

        if (anyImGui) {
            switch (event.type) {
                case InputEvent::MouseClick:
                    if (event.state == InputBegan)
                        goto POP;
                    break;
                case InputEvent::MouseMovement:
                    if (event.state == InputBegan || event.state == InputChanged)
                        goto POP;
                    break;
                case InputEvent::MouseWheel:
                    goto POP;
                    break;
                default:
                    break;
            }
        }

        {
        const char* input_state = INPUT_STATE_MAP[event.state];
        const char* input_signal = INPUT_SIGNAL_MAP[event.state];

        std::string key_code_name = "Unknown";
        std::string user_input_type_name = "None";

        std::shared_ptr<rbxInstance> input_object = input_object_array[array_index];
        if (!input_object) {
            input_object = newInstance(L, "InputObject");
            input_object_array[array_index] = input_object;

            auto& key_code = getInstanceValue<EnumItem*>(input_object, "KeyCode");
            auto& user_input_type = getInstanceValue<EnumItem*>(input_object, "UserInputType");

            switch (event.type) {
                case InputEvent::MouseClick:
                    global_mouse_wheel = 0;
                    user_input_type_name.assign(MOUSE_BUTTON_MAP[event.mouse_click.mouse]);
                    break;
                case InputEvent::MouseMovement:
                    user_input_type_name.assign("MouseMovement");
                    break;
                case InputEvent::MouseWheel:
                    user_input_type_name.assign("MouseWheel");
                    break;
                case InputEvent::Keyboard:
                    key_code_name.assign(event.keyboard.keycode);
                    user_input_type_name.assign("Keyboard");
                    break;
            }

            key_code = &Enum::enum_map.at("KeyCode").item_map.at(key_code_name);
            user_input_type = &Enum::enum_map.at("UserInputType").item_map.at(user_input_type_name);
        }
        last_input = input_object;

        Vector3 position = {
            mouse_position.x,
            mouse_position.y,
            static_cast<float>(global_mouse_wheel)
        };
        Vector3 delta = {
            mouse_delta.x,
            mouse_delta.y,
            0
        };

        setInstanceValue(input_object, L, "UserInputState", &Enum::enum_map.at("UserInputState").item_map.at(input_state));
        setInstanceValue(input_object, L, "Position", position);
        setInstanceValue(input_object, L, "Delta", delta);

        // TODO: gameProcessedEvent
        const static bool game_processed = false;

        bool has_instance = false;
        std::shared_ptr<rbxInstance> event_instance;
        if (auto ptr = event.instance.lock()) {
            event_instance = ptr;
            has_instance = true;
        } else
            event_instance = ServiceProvider::service_map.at("UserInputService");

        genericFireInputObject(L, event_instance, input_signal, input_object, game_processed);

        if (event.type == InputEvent::Keyboard) {
            const char* mouse_event = event.state == InputBegan ? "KeyDown" : event.state == InputEnded ? "KeyUp" : nullptr;
            char ch = getKeyCodeCharacter(L, shift, event.keyboard.key);
            if (mouse_event)
                genericFire(L, rbxPlayer::localmouse, mouse_event, [&L, ch] () {
                    luaL_Strbuf buf;
                    luaL_buffinit(L, &buf);
                    luaL_addchar(&buf, ch);
                    luaL_pushresult(&buf);
                    return 1;
                });
        } else if (event.type == InputEvent::MouseMovement)
            genericFire(L, rbxPlayer::localmouse, "Move");

        auto hovered_gui_objects = getGuiObjectsHovered();

        const bool fire_on_all_hovered = event.type == InputEvent::MouseMovement ? event.state == InputChanged : false;

        if (fire_on_all_hovered)
            for (size_t i = 0; i < hovered_gui_objects.size(); i++) {
                auto instance = hovered_gui_objects[i].lock();
                if (!instance || instance == event_instance)
                    continue;
                genericFireInputObject(L, instance, input_signal, input_object, game_processed);
            }
        else {
            std::weak_ptr<rbxInstance> instance_to_use;
            if (event.state == InputEnded)
                instance_to_use = last_topmost;
            else if (event.state == InputBegan)
                instance_to_use = topmost;

            if (auto i = instance_to_use.lock(); i && i != event_instance)
                genericFireInputObject(L, i, input_signal, input_object, game_processed);
        }

        auto clickable = getClickableGuiObject().lock();
        if (clickable) {
            if (event.type == InputEvent::MouseClick) {
                const auto mouse = event.mouse_click.mouse;

                const char* click_step1_name = mouse ? "internal_Click2Step1" : "internal_Click1Step1";

                if (event.state == InputEnded && mouse < 2 && getInstanceValue<bool>(clickable, click_step1_name)) {
                    std::string button_signal = "MouseButton";
                    button_signal.reserve(17);

                    button_signal += ('1' + mouse);
                    button_signal.append("Click");
                    genericFire(L, clickable, button_signal.c_str());
                }

                std::string button_signal = "MouseButton";
                button_signal.reserve(16);

                button_signal += ('1' + mouse);
                button_signal.append(event.state == InputBegan ? "Down" : "Up");
                genericFire(L, clickable, button_signal.c_str(), [&L] {
                    lua_pushinteger(L, mouse_position.x);
                    lua_pushinteger(L, mouse_position.y);

                    return 2;
                });

                if (event.state == InputBegan) {
                    setInstanceValue(clickable, L, "internal_CanActivate", true, true);

                    setInstanceValue(clickable, L, click_step1_name, true, true);
                } else if (event.state == InputEnded) {
                    if (getInstanceValue<bool>(clickable, "internal_CanActivate")) {
                        pushFunctionFromLookup(L, fireRBXScriptSignal);
                        clickable->pushEvent(L, "Activated");

                        lua_pushinstance(L, input_object);
                        lua_pushinteger(L, getInstanceValue<int32_t>(clickable, "internal_ActivateCount"));

                        lua_call(L, 3, 0);
                        // TODO: ActivateCount should be increase every time we click it and then set to 0 only when it's been x seconds after the last click
                        setInstanceValue(clickable, L, "internal_ActivateCount", int32_t(0), true);
                    }
                }
            }
        }

        if (has_instance && event_instance->isA("GuiButton")) {
            // if this behavior seems weird, note that it's accurate!
            if (event.type == InputEvent::MouseMovement && event.state == InputEnded && !IsMouseButtonDown(0))
                setInstanceValue(event_instance, L, "internal_CanActivate", false);
        }

        if (event.type == InputEvent::MouseClick && event.state == InputEnded) {
            // std::lock_guard lock(rbxInstance::instance_list_mutex);

            for (size_t i = 0; i < rbxInstance::instance_list.size(); i++) {
                auto instance = rbxInstance::instance_list[i].lock();
                if (!instance)
                    continue;

                if (!instance->isA("GuiButton"))
                    continue;

                setInstanceValue(instance, L, "internal_Click1Step1", false, true);
                setInstanceValue(instance, L, "internal_Click2Step1", false, true);
            }
        }

        if (event.type == InputEvent::MouseClick && event.state == InputBegan) {
            if (topmost && topmost->isA("TextBox"))
                captureTextBoxFocus(L, topmost);
            else if (auto previous_focused = focused_textbox.lock()) {
                focused_textbox.reset();
                releaseTextBoxFocus(L, previous_focused, false);
            }
        }

        }

        POP:
        input_event_queue.pop();
    }

    last_topmost = topmost;
}

#undef keyShifted

namespace rbxInstance_UserInputService_methods {
    static int getFocusedTextBox(lua_State* L) {
        lua_checkinstance(L, 1, "UserInputService");

        lua_pushinstance(L, focused_textbox.lock());

        return 1;
    }
    static int getKeysPressed(lua_State* L) {
        lua_checkinstance(L, 1, "UserInputService");

        lua_newtable(L);

        int index = 0;
        for (unsigned int key = 0; key < MAX_KEYBOARD_KEYS; key++) {
            const bool down = IsKeyDown(key);

            if (!down)
                continue;

            std::shared_ptr<rbxInstance> input_object = input_object_array[key];

            lua_pushinstance(L, input_object);
            lua_rawseti(L, -2, ++index);
        }

        return 1;
    }
    static int getLastInputType(lua_State *L) {
        lua_checkinstance(L, 1, "UserInputService");

        if (auto ptr = last_input.lock())
            pushEnumItem(L, getInstanceValue<EnumItem*>(ptr, "UserInputType"));
        else
            lua_pushnil(L);

        return 1;
    }
    static int getMouseDelta(lua_State* L) {
        lua_checkinstance(L, 1, "UserInputService");

        return pushVector2(L, UserInputService::mouse_delta);
    }
    static int getMouseLocation(lua_State* L) {
        lua_checkinstance(L, 1, "UserInputService");

        return pushVector2(L, UserInputService::mouse_position);
    }
    static int getMouseButtonsPressed(lua_State* L) {
        lua_checkinstance(L, 1, "UserInputService");

        lua_newtable(L);

        int index = 0;
        for (unsigned int mouse = 0; mouse < 3; mouse++) {
            const bool down = IsMouseButtonDown(mouse);

            if (!down)
                continue;

            std::shared_ptr<rbxInstance> input_object = input_object_array[MAX_KEY + 1 + mouse];

            lua_pushinstance(L, input_object);
            lua_rawseti(L, -2, ++index);
        }

        return 1;
    }
    static int isKeyDown(lua_State* L) {
        lua_checkinstance(L, 1, "UserInputService");

        EnumItem* enum_item = lua_checkenumitem(L, 2, "KeyCode");
        bool is_down = false;
        for (const auto& pair : raylib_key_to_keycode_map) {
            if (strequal(pair.second, enum_item->name.c_str())) {
                is_down = IsKeyDown(pair.first);
                break;
            }
        }

        lua_pushboolean(L, is_down);
        return 1;
    }
    static int isMouseButtonPressed(lua_State* L) {
        lua_checkinstance(L, 1, "UserInputService");

        luaL_argcheck(L, lua_isnumber(L, 2) || lua_isuserdata(L, 2), 2, "expected number or userdata");

        int mouse = -1;
        if (lua_isnumber(L, 2))
            mouse = luaL_checkinteger(L, 2);
        else 
            mouse = lua_checkenumitem(L, 2, "UserInputType")->value;

        if (mouse > 2) {
            getTask(L)->console->warning("UserInputService.IsMouseButtonPressed - UserInputType provided is not a mouse button.");
            lua_pushboolean(L, false);
        } else
            lua_pushboolean(L, IsMouseButtonPressed(mouse));

        return 1;
    }
}; // namespace rbxInstance_UserInputService_methods

void rbxInstance_UserInputService_init() {
    UserInputService::signalMouseMovement(nullptr, InputBegan);

    rbxClass::class_map["UserInputService"]->methods["GetFocusedTextBox"].func = rbxInstance_UserInputService_methods::getFocusedTextBox;
    rbxClass::class_map["UserInputService"]->methods["GetKeysPressed"].func = rbxInstance_UserInputService_methods::getKeysPressed;
    rbxClass::class_map["UserInputService"]->methods["GetLastInputType"].func = rbxInstance_UserInputService_methods::getLastInputType;
    rbxClass::class_map["UserInputService"]->methods["GetMouseDelta"].func = rbxInstance_UserInputService_methods::getMouseDelta;
    rbxClass::class_map["UserInputService"]->methods["GetMouseLocation"].func = rbxInstance_UserInputService_methods::getMouseLocation;
    rbxClass::class_map["UserInputService"]->methods["GetMouseButtonsPressed"].func = rbxInstance_UserInputService_methods::getMouseButtonsPressed;
    rbxClass::class_map["UserInputService"]->methods["IsKeyDown"].func = rbxInstance_UserInputService_methods::isKeyDown;
    rbxClass::class_map["UserInputService"]->methods["IsMouseButtonPressed"].func = rbxInstance_UserInputService_methods::isMouseButtonPressed;
}

}; // namespace frostbyte
