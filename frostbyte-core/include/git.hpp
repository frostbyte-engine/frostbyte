#pragma once

#include <optional>
#include <string>

namespace frostbyte {

void gitInit();
void gitShutdown();

std::optional<std::string> gitShallowClone(const char* url, const char* output_path);

};

