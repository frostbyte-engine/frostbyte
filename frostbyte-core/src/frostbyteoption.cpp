#include "frostbyteoption.hpp"
#include "engine/classes/frostbyte/frostbyteoptions.hpp"

namespace frostbyte {

FrostbyteOption::FrostbyteOption(bool def): value(def) { }

void FrostbyteOption::setValue(bool new_value) {
    value = new_value;
    for (auto& callback : on_set_list)
        callback.callback(new_value, callback.userdata);

    if (frostbyteservice_option_name)
        frostbyte::option_update_list.push_back(FrostbyteOptionUpdate{ frostbyteservice_option_name, value });
};

};



