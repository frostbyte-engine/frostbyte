#pragma once

#include "lua.h"

namespace frostbyte {
    namespace userdata {
        #define USERDATA_TYPES         \
            X(Color3)                  \
            X(ColorSequenceKeypoint)   \
            X(ColorSequence)           \
            X(Content)                 \
            X(Enums)                   \
            X(Enum)                    \
            X(EnumItem)                \
            X(Font)                    \
            X(Instance)                \
            X(NumberRange)             \
            X(NumberSequenceKeypoint)  \
            X(NumberSequence)          \
            X(RBXScriptConnection)     \
            X(RBXScriptSignal)         \
            X(Rect)                    \
            X(TweenInfo)               \
            X(UDim)                    \
            X(UDim2)                   \
            X(Vector2)                 \
            X(Vector3)                 \
                                       \
            X(InstructionWrapper)      \
            X(DrawEntry)

        #define X(name) name,
        enum UserdataTag {
            SharedPtrObject,
            USERDATA_TYPES
        };
        #undef X

        extern const char* const userdata_names[];

        bool get(lua_State* L, void*& out, int ud, int ttag);
        bool is(lua_State* L, int ud, int ttag);
        void* check(lua_State* L, int ud, int ttag);

        int newClassMetatable(lua_State* L, int ttag);
        int getClassMetatable(lua_State* L, int ttag);
    }
}
