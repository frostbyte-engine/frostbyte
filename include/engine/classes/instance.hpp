#pragma once

#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <shared_mutex>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include "common.hpp"
#include "raylib.h"

#include "engine/datatypes/brickcolor.hpp"
#include "engine/datatypes/colorsequence.hpp"
#include "engine/datatypes/enum.hpp"
#include "engine/datatypes/font.hpp"
#include "engine/datatypes/numberrange.hpp"
#include "engine/datatypes/numbersequence.hpp"
#include "engine/datatypes/rect.hpp"
#include "engine/datatypes/tweeninfo.hpp"
#include "engine/datatypes/udim.hpp"
#include "engine/datatypes/udim2.hpp"

#include "lua.h"

// Instance is the base class of all objects. We may adapt to Roblox's choice of an Object abstraction in the future.

namespace frostbyte {

class rbxProperty;

enum TypeCategory {
    Primitive,
    DataType,
    Instance
};
enum Type_PrimitiveType {
    Boolean,
    Integer32,
    Integer64,
    Float,
    Double,
    String,
    Nil,
};

class rbxClass;

struct rbxMethod {
    std::shared_ptr<rbxClass> _class;
    std::string name;
    std::optional<std::string> route = std::nullopt;
    lua_CFunction func = nullptr;
    lua_Continuation cont = nullptr;
};

class rbxInstance;
class rbxValue;

class rbxClass {
public:
    static std::map<std::string, std::shared_ptr<rbxClass>> class_map;
    static std::vector<std::string> valid_class_names;
    static std::vector<std::string> valid_services;

    std::string name;

    enum Tags : uint8_t {
        NotCreatable = 1 << 0,
    };
    uint8_t tags = 0;

    std::shared_ptr<rbxClass> superclass;
    std::map<std::string, std::shared_ptr<rbxProperty>> properties;
    std::map<std::string, rbxMethod> methods;
    std::vector<std::string> events;
    std::function<void(lua_State* L, std::shared_ptr<rbxInstance> instance)> constructor = nullptr;
    std::function<void(rbxInstance*)> destructor = nullptr;

    void newMethod(const char* name, lua_CFunction func, lua_Continuation cont = nullptr) {
        rbxMethod method;
        method.name = name;
        method.func = func;
        method.cont = cont;
        methods.try_emplace(name, method);
    }

    std::shared_ptr<rbxProperty> newInternalProperty(const char* name, TypeCategory type_category, rbxValue default_value);
};

struct rbxCallback {
    int index; // index in method lookup; -1 means empty (nil)
};

typedef std::variant<
    std::monostate,
    bool,
    int32_t,
    int64_t,
    float,
    double,
    std::string,

    rbxCallback,

    // TODO: this should most likely be a weak_ptr
    std::shared_ptr<rbxInstance>,

    BrickColor*,
    Color,
    EnumItem*,
    EngineFont,
    TweenInfo,
    ColorSequenceKeypoint,
    ColorSequence,
    NumberRange,
    NumberSequenceKeypoint,
    NumberSequence,
    Rect,
    UDim,
    UDim2,
    Vector2,
    Vector3
> rbxValueVariant;

class rbxValue {
public:
    // TODO: setscriptable applies to all instances of a class because each value's property member will point to the same single property shared ptr.
    // We could get around this by making property an rbxProperty not a shared ptr, but this comes at the cost of memory.
    // Alternatively, we could just give rbxValue its own tags field that initially copies its property.
    std::shared_ptr<rbxProperty> property;

    rbxValueVariant value;
};

std::vector<std::weak_ptr<rbxInstance>> getNilInstances();

class rbxInstance {
public:
    static std::vector<std::weak_ptr<rbxInstance>> instance_list;
    // static std::shared_mutex instance_list_mutex;

    std::shared_ptr<rbxClass> _class;
    std::map<std::string, rbxValue> values;
    std::map<std::string, rbxMethod> methods;
    std::vector<std::string> events;
    std::vector<std::shared_ptr<rbxInstance>> children;
    std::map<std::string, rbxValueVariant> attributes;

    // std::shared_mutex values_mutex;
    // std::shared_mutex children_mutex;

    bool destroyed = false;
    bool parent_locked = false;
    void* userdata = nullptr;

    // std::shared_mutex destroyed_mutex;
    // std::shared_mutex parent_locked_mutex;

    // static lua_State* destructorL;

    rbxInstance(std::shared_ptr<rbxClass> _class);
    ~rbxInstance();

    bool isA(rbxClass* target_class);
    bool isA(const char* class_name);
    int pushEvent(lua_State* L, const char* name);

    rbxValueVariant& getValueVariant(const char* name);
    template<typename T>
    T& getValue(const char* name) {
        return std::get<T>(getValueVariant(name));
    }

    std::shared_ptr<rbxInstance> findFirstAncestor(const char* name);
    std::shared_ptr<rbxInstance> findFirstAncestorOfClass(const char* classname);
    std::shared_ptr<rbxInstance> findFirstAncestorWhichIsA(const char* classname);
    std::shared_ptr<rbxInstance> findFirstChild(const char* name, bool recursive = false);
    std::shared_ptr<rbxInstance> findFirstChildWhichIsA(const char* classname);
};

#define PROP_INSTANCE_ARCHIVABLE "Archivable"
#define PROP_INSTANCE_NAME "Name"
#define PROP_INSTANCE_CLASS_NAME "ClassName"
#define PROP_INSTANCE_PARENT "Parent"

#define METHOD_INSTANCE_DESTROY "Destroy"

void reportChanged(lua_State* L, std::shared_ptr<rbxInstance> instance, const char* property);
void destroyInstance(lua_State* L, std::shared_ptr<rbxInstance> instance, bool dont_remove_from_old_parent_children = false);
int pushInstanceValue(lua_State* L, rbxValueVariant& value);
void setInstanceParent(lua_State* L, std::shared_ptr<rbxInstance> instance, std::shared_ptr<rbxInstance> new_parent, bool dont_remove_from_old_parent_children = false, bool dont_set_value = false);

bool isDescendantOf(std::shared_ptr<rbxInstance> other);

class rbxProperty {
public:
    enum Tags : uint8_t {
        Hidden = 1 << 0,
        Deprecated = 1 << 1,
        ReadOnly = 1 << 2,
        WriteOnly = 1 << 3,
        NotScriptable = 1 << 4
    };
    uint8_t tags = 0;
    bool internal = false;

    TypeCategory type_category;
    std::string type_name;
    rbxValue default_value;

    std::optional<std::string> route = std::nullopt;
};

rbxValueVariant& getInstanceValueVariant(std::shared_ptr<rbxInstance> instance, const char* name);

template<typename T>
T& getInstanceValue(std::shared_ptr<rbxInstance> instance, const char* name) {
    return instance->getValue<T>(name);
}

rbxValueVariant luaValueToValueVariant(lua_State* L, int idx, rbxValueVariant& reference);
rbxValueVariant luaValueToValueVariant(lua_State* L, int idx);

template<typename T>
int setInstanceValueVariant(rbxValueVariant& variant, T value) {
    if (std::holds_alternative<BrickColor*>(variant)) {
        if constexpr (std::is_same_v<T, BrickColor*>) {
            auto& v = std::get<BrickColor*>(variant);
            if (v == value)
                return 1;

            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting BrickColor value");
    } else if (std::holds_alternative<Color>(variant)) {
        if constexpr (std::is_same_v<T, Color>) {
            auto& v = std::get<Color>(variant);
            if (value.r == v.r && value.g == v.g && value.b == v.b)
                return 1;

            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting Color value");
    } else if (std::holds_alternative<EnumItem*>(variant)) {
        if constexpr (std::is_same_v<T, EnumItem*>) {
            auto& v = std::get<EnumItem*>(variant);
            if (value == v)
                return 1;

            v = value;

            return 0;
        }
        throw std::runtime_error("expected EnumItem* when setting an EnumItem");
    } else if (std::holds_alternative<EngineFont>(variant)) {
        if constexpr (std::is_same_v<T, EngineFont>) {
            auto& v = std::get<EngineFont>(variant);
            if (v.font && v.font == value.font)
                return 1;

            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting an EngineFont");
    } else if (std::holds_alternative<TweenInfo>(variant)) {
        if constexpr (std::is_same_v<T, TweenInfo>) {
            auto& v = std::get<TweenInfo>(variant);
            if (value.easing_direction == v.easing_direction && value.time == v.time && value.delay_time == v.delay_time && value.repeat_count == v.repeat_count && value.easing_style == v.easing_style && value.reverses == v.reverses)
                return 1;
            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting TweenInfo value");
    } else if (std::holds_alternative<ColorSequenceKeypoint>(variant)) {
        if constexpr (std::is_same_v<T, ColorSequenceKeypoint>) {
            auto& v = std::get<ColorSequenceKeypoint>(variant);
            if (value.time == v.time && value.value.r == v.value.r && value.value.g == v.value.g && value.value.b == v.value.b)
                return 1;
            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting ColorSequenceKeypoint value");
    } else if (std::holds_alternative<ColorSequence>(variant)) {
        if constexpr (std::is_same_v<T, ColorSequence>) {
            auto& v = std::get<ColorSequence>(variant);
            // FIXME: colorsequence equality
            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting ColorSequence value");
    } else if (std::holds_alternative<NumberRange>(variant)) {
        if constexpr (std::is_same_v<T, NumberRange>) {
            auto& v = std::get<NumberRange>(variant);
            if (value.min == v.min && value.max == v.max)
                return 1;
            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting NumberRange value");
    } else if (std::holds_alternative<NumberSequenceKeypoint>(variant)) {
        if constexpr (std::is_same_v<T, NumberSequenceKeypoint>) {
            auto& v = std::get<NumberSequenceKeypoint>(variant);
            if (value.envelope == v.envelope && value.time == v.time && value.value == v.value)
                return 1;
            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting NumberSequenceKeypoint value");
    } else if (std::holds_alternative<NumberSequence>(variant)) {
        if constexpr (std::is_same_v<T, NumberSequence>) {
            auto& v = std::get<NumberSequence>(variant);
            // FIXME: numbersequence equality
            // if (value.envelope == v.envelope && value.time == v.time && value.value == v.value)
            //     return 1;
            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting NumberSequence value");
    } else if (std::holds_alternative<Rect>(variant)) {
        if constexpr (std::is_same_v<T, Rect>) {
            auto& v = std::get<Rect>(variant);
            if (value.minx == v.minx && value.miny == v.miny && value.maxx == v.maxx && value.maxy == v.maxy)
                return 1;
            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting Rect value");
    } else if (std::holds_alternative<UDim>(variant)) {
        if constexpr (std::is_same_v<T, UDim>) {
            auto& v = std::get<UDim>(variant);
            if (value.scale == v.scale && value.offset == v.offset)
                return 1;
            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting UDim value");
    } else if (std::holds_alternative<UDim2>(variant)) {
        if constexpr (std::is_same_v<T, UDim2>) {
            auto& v = std::get<UDim2>(variant);
            if (value.x.scale == v.x.scale && value.x.offset == v.x.offset && value.y.scale == v.y.scale && value.y.offset == v.y.offset)
                return 1;
            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting UDim2 value");
    } else if (std::holds_alternative<Vector2>(variant)) {
        if constexpr (std::is_same_v<T, Vector2>) {
            auto& v = std::get<Vector2>(variant);
            if (value.x == v.x && value.y == v.y)
                return 1;
            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting Vector2 value");
    } else if (std::holds_alternative<Vector3>(variant)) {
        if constexpr (std::is_same_v<T, Vector3>) {
            auto& v = std::get<Vector3>(variant);
            if (value.x == v.x && value.y == v.y && value.z == v.z)
                return 1;
            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting Vector3 value");
    } else if (std::holds_alternative<std::shared_ptr<rbxInstance>>(variant)) {
        if constexpr (std::is_same_v<T, std::shared_ptr<rbxInstance>>) {
            auto& v = std::get<std::shared_ptr<rbxInstance>>(variant);
            if (value == v)
                return 1;

            v = value;

            return 0;
        }
        throw std::runtime_error("unexpected type when setting rbxInstance value");
    } else if (std::holds_alternative<rbxCallback>(variant))
        throw std::runtime_error("values of type rbxCallback cannot be set via setValue");

    #define handleType(type) {                                                             \
        if (std::holds_alternative<type>(variant)) {                                       \
            if constexpr (std::is_same_v<T, type>) {                                       \
                if (value == std::get<type>(variant))                                      \
                    return 1;                                                              \
            } else                                                                         \
                throw std::runtime_error("unexpected type when setting " #type " value");  \
        }                                                                                  \
    }

    handleType(bool)
    handleType(int32_t)
    handleType(int64_t)
    handleType(float)
    handleType(double)
    handleType(std::string)

    #undef handleType

    return -1;
}
template<typename T>
void setInstanceValue(std::shared_ptr<rbxInstance> instance, lua_State* L, const char* name, T value, bool dont_report_changed = false) {
    // std::unique_lock lock(instance->values_mutex);

    auto& rbxvalue = instance->values.at(name);
    auto& variant = rbxvalue.value;

    if (std::holds_alternative<std::shared_ptr<rbxInstance>>(variant)) {
        if constexpr (std::is_same_v<T, std::shared_ptr<rbxInstance>>) {
            auto& v = std::get<std::shared_ptr<rbxInstance>>(variant);
            if (value == v)
                goto DUPLICATE;

            if (strequal(name, PROP_INSTANCE_PARENT)) {
                // lock.unlock();
                setInstanceParent(L, instance, value, false, true);
                // lock.lock();
            }

            v = value;

            goto AFTER_SET;
        }
        throw std::runtime_error("unexpected type when setting rbxInstance value");
    } else {
        int result = setInstanceValueVariant(variant, value);
        if (result == 1)
            goto DUPLICATE;
        else if (result == 0)
            goto AFTER_SET;
    }

    std::get<T>(variant) = value;

    AFTER_SET:

    // lock.unlock();

    if (!rbxvalue.property->internal && !dont_report_changed)
        reportChanged(L, instance, name);

    DUPLICATE: ;
}

void setInstanceValueFromVariant(std::shared_ptr<rbxInstance> instance, lua_State* L, const char* name, rbxValueVariant value, bool dont_report_changed = false);


bool lua_isinstance(lua_State* L, int narg);
std::shared_ptr<rbxInstance>& lua_checkinstance(lua_State* L, int narg, const char* class_name = nullptr);
std::shared_ptr<rbxInstance> lua_optinstance(lua_State* L, int narg, const char* class_name = nullptr);

void rbxInstanceSetup(lua_State* L, std::string api_jump);
void rbxInstanceCleanup(lua_State* L);

std::shared_ptr<rbxInstance> newInstance(lua_State* L, const char* class_name, std::shared_ptr<rbxInstance> parent = nullptr);
std::shared_ptr<rbxInstance> cloneInstance(lua_State* L, std::shared_ptr<rbxInstance> reference, bool is_deep = true, std::optional<std::map<std::shared_ptr<rbxInstance>, std::shared_ptr<rbxInstance>>*> cloned_map = std::nullopt);
int lua_pushinstance(lua_State* L, std::shared_ptr<rbxInstance> instance);

namespace rbxInstance_datatype {
    int _new(lua_State* L);
    int from_existing(lua_State* L);
}; // namespace rbxInstance_datatype

extern std::shared_ptr<rbxInstance> hiddenui;

}; // namespace frostbyte
