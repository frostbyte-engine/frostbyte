#pragma once

#include <cstddef>

#ifdef __EMSCRIPTEN__
typedef int CURLcode;
#else
#include "curl/curl.h"
#endif

namespace frostbyte {

void httpInit();
void httpShutdown();

typedef struct {
    char *memory;
    size_t size;
    CURLcode res;
} MemoryStruct;
void newGetRequest(const char* url, MemoryStruct* chunk);

};
