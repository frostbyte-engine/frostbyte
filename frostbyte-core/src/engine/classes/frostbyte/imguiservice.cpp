#include "engine/classes/frostbyte/imguiservice.hpp"

#include "common.hpp"
#include "ui/ui.hpp"

#include "engine/datatypes/rbxscriptsignal.hpp"
#include "engine/classes/instance.hpp"
#include "engine/classes/serviceprovider.hpp"

#ifndef FROSTBYTE_HEADLESS
#include "imgui.h"
#endif

#include "lapi.h"
#include "lobject.h"
#include "lualib.h"
#include "ltable.h"

#include <memory>

namespace frostbyte {

namespace ImGuiService_methods {
    #ifdef FROSTBYTE_HEADLESS
    static int begin(lua_State* L) {
        luaL_checkstring(L, 1);
        lua_optinstance(L, 2, "BoolValue");

        lua_pushboolean(L, false);
        return 1;
    }
    static int end(lua_State* L) {
        return 0;
    }

    static int text(lua_State* L) {
        luaL_checkstring(L, 1);
        return 0;
    }

    static int button(lua_State* L) {
        luaL_checkstring(L, 1);

        lua_pushboolean(L, false);
        return 1;
    }
    static int checkbox(lua_State* L) {
        luaL_checkstring(L, 1);
        lua_checkinstance(L, 2, "BoolValue");

        lua_pushboolean(L, false);
        return 1;
    }
    static int bullet(lua_State* L) {
        return 0;
    }

    static int beginCombo(lua_State* L) {
        luaL_checkstring(L, 1);

        lua_pushboolean(L, false);
        return 1;
    }
    static int endCombo(lua_State* L) {
        return 0;
    }
    static int combo(lua_State* L) {
        luaL_checkstring(L, 1);
        lua_checkinstance(L, 2, "IntValue");
        luaL_checktype(L, 3, LUA_TTABLE);

        lua_pushboolean(L, false);
        return 1;
    }

    static int inputText(lua_State* L) {
        luaL_checkstring(L, 1);
        lua_checkinstance(L, 2, "StringValue");

        lua_pushboolean(L, false);
        return 1;
    }

    static int colorEdit(lua_State* L) {
        luaL_checkstring(L, 1);
        lua_checkinstance(L, 2, "Color3Value");

        lua_pushboolean(L, false);
        return 1;
    }
    #else
    static int begin(lua_State* L) {
        const char* str = luaL_checkstring(L, 1);
        std::shared_ptr<rbxInstance> boolvalue = lua_optinstance(L, 2, "BoolValue");

        bool* p_open = NULL;
        if (boolvalue)
            p_open = &getInstanceValue<bool>(boolvalue, "Value");

        lua_pushboolean(L, ImGui::Begin(str, p_open));
        return 1;
    }
    static int end(lua_State* L) {
        ImGui::End();

        return 0;
    }

    static int text(lua_State* L) {
        size_t textl;
        const char* text = luaL_checklstring(L, 1, &textl);

        ImGui::Text("%.*s", static_cast<int>(textl), text);

        return 0;
    }

    static int button(lua_State* L) {
        const char* label = luaL_checkstring(L, 1);

        lua_pushboolean(L, ImGui::Button(label));
        return 1;
    }
    static int checkbox(lua_State* L) {
        const char* label = luaL_checkstring(L, 1);
        std::shared_ptr<rbxInstance> boolvalue = lua_checkinstance(L, 2, "BoolValue");

        bool changed = ImGui::Checkbox(label, &getInstanceValue<bool>(boolvalue, "Value"));
        if (changed)
            reportChanged(L, boolvalue, "Value");

        lua_pushboolean(L, changed);
        return 1;
    }
    static int bullet(lua_State* L) {
        ImGui::Bullet();
        return 0;
    }

    static int beginCombo(lua_State* L) {
        const char* label = luaL_checkstring(L, 1);

        lua_pushboolean(L, ImGui::BeginCombo(label, label));
        return 1;
    }
    static int endCombo(lua_State* L) {
        ImGui::EndCombo();
        return 0;
    }
    static int combo(lua_State* L) {
        const char* label = luaL_checkstring(L, 1);
        std::shared_ptr<rbxInstance> intvalue = lua_checkinstance(L, 2, "IntValue");
        luaL_checktype(L, 3, LUA_TTABLE);

        LuaTable* table = hvalue(luaA_toobject(L, 3));
        const int item_count = luaH_getn(table);

        const char** items = static_cast<const char**>(malloc(sizeof(const char*) * item_count));
        for (int i = 0; i < item_count; i++) {
            auto value = &table->array[i];
            if (value->tt != LUA_TSTRING)
                luaL_error(L, "invalid value at index %d! expected string, got %s", i + 1, lua_typename(L, value->tt));
            items[i] = svalue(value);
        }

        int64_t* value_ptr = &getInstanceValue<int64_t>(intvalue, "Value");
        int32_t value = *value_ptr;

        const bool changed = ImGui::Combo(label, &value, items, item_count);
        free(items);

        if (changed) {
            *value_ptr = value;
            reportChanged(L, intvalue, "Value");
        }

        lua_pushboolean(L, changed);
        return 1;
    }

    static int inputText(lua_State* L) {
        const char* label = luaL_checkstring(L, 1);
        std::shared_ptr<rbxInstance> stringvalue = lua_checkinstance(L, 2, "StringValue");

        bool changed = ImGui_STDString(label, getInstanceValue<std::string>(stringvalue, "Value"));
        if (changed)
            reportChanged(L, stringvalue, "Value");

        lua_pushboolean(L, changed);
        return 1;
    }

    static int colorEdit(lua_State* L) {
        const char* label = luaL_checkstring(L, 1);
        std::shared_ptr<rbxInstance> color3value = lua_checkinstance(L, 2, "Color3Value");

        Color* color = &getInstanceValue<Color>(color3value, "Value");

        float col[3] = { color->r / 255.f, color->g / 255.f, color->b / 255.f };

        const bool changed = ImGui::ColorEdit3(label, col);

        if (changed) {
            color->r = col[0] * 255.f;
            color->g = col[1] * 255.f;
            color->b = col[2] * 255.f;
            reportChanged(L, color3value, "Value");
        }

        lua_pushboolean(L, changed);
        return 1;
    }
    #endif
}

std::shared_ptr<rbxInstance> ImGuiService;

void ImGuiService_init(lua_State* L, std::shared_ptr<rbxInstance> datamodel) {
    auto this_class = std::make_shared<rbxClass>();
    this_class->name.assign("ImGuiService");
    this_class->tags |= rbxClass::NotCreatable;
    this_class->superclass = rbxClass::class_map.at("Instance");

    this_class->newMethod("Begin", ImGuiService_methods::begin);
    this_class->newMethod("End", ImGuiService_methods::end);

    this_class->newMethod("Text", ImGuiService_methods::text);

    this_class->newMethod("Button", ImGuiService_methods::button);
    this_class->newMethod("Checkbox", ImGuiService_methods::checkbox);
    this_class->newMethod("Bullet", ImGuiService_methods::bullet);

    this_class->newMethod("BeginCombo", ImGuiService_methods::beginCombo);
    this_class->newMethod("EndCombo", ImGuiService_methods::endCombo);
    this_class->newMethod("Combo", ImGuiService_methods::combo);

    this_class->newMethod("InputText", ImGuiService_methods::inputText);

    this_class->newMethod("ColorEdit", ImGuiService_methods::colorEdit);

    this_class->events.push_back(rbxEvent{ .name = "Render" });

    rbxClass::class_map.try_emplace("ImGuiService", this_class);
    ServiceProvider::registerService("ImGuiService");

    ImGuiService = ServiceProvider::getService(L, datamodel, "ImGuiService");
}

void ImGuiService_render(lua_State *L) {
    pushFunctionFromLookup(L, fireRBXScriptSignal);
    ImGuiService->pushSignal(L, "Render", true);

    lua_call(L, 1, 0);
}

}; // namespace frostbyte
