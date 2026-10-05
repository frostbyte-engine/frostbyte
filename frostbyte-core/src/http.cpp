#include "http.hpp"

#include <cstdio>
#include <cstdlib>
#include <cstring>

namespace frostbyte {

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <cstdlib>

void httpInit() {}
void httpShutdown() {}

EM_ASYNC_JS(int, js_fetch, (const char* url, const char* method, const char* headersJson), {
    try {
        const r = await fetch(UTF8ToString(url), {
            method: UTF8ToString(method),
            headers: JSON.parse(UTF8ToString(headersJson))
        });
        Module._resp = new Uint8Array(await r.arrayBuffer());
        return r.status;
    } catch (e) {
        console.error('fetch failed:', e);
        Module._resp = new Uint8Array(0);
        return -1;
    }
});

EM_JS(int, js_resp_len, (), { return Module._resp.length; });
EM_JS(void, js_resp_copy, (char* dst), { HEAPU8.set(Module._resp, dst); });

void performRequest(const char* url, MemoryStruct* chunk, const char* method = "GET") {
    chunk->memory = static_cast<char*>(malloc(1));
    chunk->size = 0;

    // TODO: can we set User-Agent?
    const char* headersJson =
        "{\"Roblox-Game-Id\": \"abcdefg\","
        "\"Roblox-Session-Id\": \"{\\\"GameId\\\": abcdefg}\"}";

    int status = js_fetch(url, method, headersJson);
    if (status < 0) {
        chunk->res = 7; // CURLE_COULDNT_CONNECT
        return;
    }

    int len = js_resp_len();
    char* mem = static_cast<char*>(realloc(chunk->memory, len + 1));
    if (!mem) {
        chunk->res = 27; // CURLE_OUT_OF_MEMORY
        return;
    }
    chunk->memory = mem;
    if (len > 0)
        js_resp_copy(chunk->memory);
    chunk->memory[len] = 0;
    chunk->size = len;
    chunk->res = CURLE_OK;
}

void newGetRequest(const char* url, MemoryStruct* chunk) {
    performRequest(url, chunk);
}

#else
void httpInit() {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}
void httpShutdown() {
    curl_global_cleanup();
}
/* credits START: https://stackoverflow.com/questions/27007379/how-do-i-get-response-value-using-curl-in-c/27007490#27007490 */

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    MemoryStruct *mem = (MemoryStruct *)userp;

    mem->memory = static_cast<char*>(realloc(mem->memory, mem->size + realsize + 1));
    if(mem->memory == NULL) {
        fprintf(stderr, "not enough memory (realloc returned NULL)\n");
        return 0;
    }

    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

void performRequest(CURL* curl, MemoryStruct* chunk, const char* method = "GET") {
    CURLcode res;

    chunk->memory = static_cast<char*>(malloc(1));
    chunk->size = 0;

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*) chunk);

    struct curl_slist* headers = NULL;
    headers = curl_slist_append(headers, "User-Agent: Roblox/WinInet");
    headers = curl_slist_append(headers, "Roblox-Game-Id: abcdefg");
    headers = curl_slist_append(headers, "Roblox-Session-Id: {\"GameId\": abcdefg}");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, method);

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    chunk->res = res;
}
/* credits END: https://stackoverflow.com/questions/27007379/how-do-i-get-response-value-using-curl-in-c/27007490#27007490 */

void newGetRequest(const char* url, MemoryStruct* chunk) {
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        return performRequest(curl, chunk);
    }
    chunk->res = CURLE_FAILED_INIT;
}
#endif

};
