#pragma once

#include <vector>

namespace frostbyte {

using frostbyte_option_onset_func = void(*)(bool, void*);
struct FrostbyteOptionCallback {
    void* userdata = nullptr;
    frostbyte_option_onset_func callback = nullptr;
};

class FrostbyteOption {
public:
    FrostbyteOption(const FrostbyteOption&) = delete;
    FrostbyteOption& operator=(const FrostbyteOption&) = delete;

    FrostbyteOption(bool def);

    bool value;
    const char* frostbyteservice_option_name = nullptr;

    operator bool() const {
        return value;
    }
    FrostbyteOption& operator=(bool new_value) {
        setValue(new_value);
        return *this; 
    }

    std::vector<FrostbyteOptionCallback> on_set_list;

    void setValue(bool new_value);
};

};


