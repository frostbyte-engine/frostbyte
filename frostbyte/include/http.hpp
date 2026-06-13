#pragma once

#include <cstddef>
#include "curl/curl.h"

namespace frostbyte {

typedef struct {
    char *memory;
    size_t size;
    CURLcode res;
} MemoryStruct;
 void newGetRequest(const char* url, MemoryStruct* chunk);

};
