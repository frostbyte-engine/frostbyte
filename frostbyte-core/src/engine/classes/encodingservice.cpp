#include "engine/classes/encodingservice.hpp"
#include "engine/classes/instance.hpp"
#include "engine/classes/serviceprovider.hpp"

#include "base64.hpp"

#include "engine/datatypes/enum.hpp"
#include "lstring.h"
#include "lua.h"
#include "lualib.h"

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <blake3.h>
#include <zstd.h>

namespace frostbyte {

EVP_MD_CTX* ctx = NULL;
blake3_hasher blake3;
EVP_MD* blake2b = NULL;
EVP_MD* md5 = NULL;
EVP_MD* sha1 = NULL;
EVP_MD* sha256 = NULL;

namespace rbxInstance_EncodingService_methods {

static int base64Decode(lua_State* L) {
    lua_checkinstance(L, 1, "EncodingService");

    size_t bufferlen;
    void* buffer = luaL_checkbuffer(L, 2, &bufferlen);

    // TODO: input validation and then error
    std::string decoded = b64_decode(static_cast<const unsigned char*>(buffer), bufferlen);

    void* newbuffer = lua_newbuffer(L, decoded.size());
    memcpy(newbuffer, decoded.c_str(), decoded.size());
    return 1;
}
static int base64Encode(lua_State* L) {
    lua_checkinstance(L, 1, "EncodingService");

    size_t bufferlen;
    void* buffer = luaL_checkbuffer(L, 2, &bufferlen);

    std::string encoded = b64_encode(static_cast<const unsigned char*>(buffer), bufferlen);

    void* newbuffer = lua_newbuffer(L, encoded.size());
    memcpy(newbuffer, encoded.c_str(), encoded.size());
    return 1;
}

static int compressBuffer(lua_State* L) {
    lua_checkinstance(L, 1, "EncodingService");

    size_t bufferlen;
    void* buffer = luaL_checkbuffer(L, 2, &bufferlen);

    auto algorithm = lua_checkenumitem(L, 3, "CompressionAlgorithm");

    int compression_level = luaL_optinteger(L, 4, 1);

    switch (algorithm->value) {
        case 0: { // Zstd
            int minlevel = -7;
            int maxlevel = ZSTD_maxCLevel();
            if (compression_level < minlevel || compression_level > maxlevel)
                luaL_error(L, "invalid compression level for zstd (expected value between %d and %d, got %d)", minlevel, maxlevel, compression_level);

            size_t capacity = ZSTD_compressBound(bufferlen);
            if (capacity > MAXSSIZE)
                luaL_error(L, "failed to compress (compress bound too big)");

            char* dst = static_cast<char*>(malloc(capacity));
            memset(dst, 0, capacity);

            size_t len = ZSTD_compress(dst, capacity, buffer, bufferlen, compression_level);
            if (ZSTD_isError(len))
                luaL_error(L, "failed to compress: %s (%zu)", ZSTD_getErrorName(len), len);

            void* newbuffer = lua_newbuffer(L, len);
            memcpy(newbuffer, dst, len);
            free(dst);
            return 1;
        }
    }
    luaL_error(L, "unhandled HashAlgorithm value %d", algorithm->value);
}
static int decompressBuffer(lua_State* L) {
    lua_checkinstance(L, 1, "EncodingService");

    size_t bufferlen;
    void* buffer = luaL_checkbuffer(L, 2, &bufferlen);

    auto algorithm = lua_checkenumitem(L, 3, "CompressionAlgorithm");

    switch (algorithm->value) {
        case 0: { // Zstd
            size_t capacity = ZSTD_getFrameContentSize(buffer, bufferlen);
            if (capacity > MAXSSIZE)
                luaL_error(L, "failed to decompress (frame content size too big, was the input really compressed?)");

            void* newbuffer = lua_newbuffer(L, capacity);

            size_t len = ZSTD_decompress(newbuffer, capacity, buffer, bufferlen);
            if (ZSTD_isError(len))
                luaL_error(L, "failed to dcompress: %s (%zu)", ZSTD_getErrorName(len), len);

            return 1;
        }
    }
    luaL_error(L, "unhandled HashAlgorithm value %d", algorithm->value);
}

static int computeBufferHash(lua_State* L) {
    lua_checkinstance(L, 1, "EncodingService");

    size_t bufferlen;
    void* buffer = luaL_checkbuffer(L, 2, &bufferlen);

    auto algorithm = lua_checkenumitem(L, 3, "HashAlgorithm");

    EVP_MD* target_libcrypto_md = NULL;
    const char* target_hash_name = nullptr;

    switch (algorithm->value) {
        case 0: // Blake2b
            if (blake2b == NULL)
                luaL_error(L, "Blake2b fetch failed");

            target_libcrypto_md = blake2b;
            target_hash_name = "blake2b";
            break;
        case 1: { // Blake3
            blake3_hasher_reset(&blake3);

            blake3_hasher_update(&blake3, buffer, bufferlen);

            void* newbuffer = lua_newbuffer(L, BLAKE3_OUT_LEN);
            blake3_hasher_finalize(&blake3, static_cast<unsigned char*>(newbuffer), BLAKE3_OUT_LEN);
            return 1;
        }
        case 2: // Md5
            if (md5 == NULL)
                luaL_error(L, "MD5 fetch failed");

            target_libcrypto_md = md5;
            target_hash_name = "md5";
            break;
        case 3: // Sha1
            if (sha1 == NULL)
                luaL_error(L, "SHA1 fetch failed");

            target_libcrypto_md = sha1;
            target_hash_name = "sha1";
            break;
        case 4: { // Sha256
            if (sha256 == NULL)
                luaL_error(L, "SHA256 fetch failed");

            target_libcrypto_md = sha256;
            target_hash_name = "sha256";
            break;
        }
        default:
            break;
    }

    if (target_libcrypto_md != NULL) {
        if (ctx == NULL)
            luaL_error(L, "libcrypto context creation failed");

        if (!EVP_DigestInit_ex(ctx, target_libcrypto_md, NULL))
            luaL_error(L, "failed to initialize %s digest", target_hash_name);

        if (!EVP_DigestUpdate(ctx, buffer, bufferlen))
            luaL_error(L, "failed to update %s digest", target_hash_name);

        unsigned char* digest = static_cast<unsigned char*>(OPENSSL_malloc(EVP_MD_get_size(target_libcrypto_md)));
        if (digest == NULL)
            luaL_error(L, "failed to allocate digest");

        unsigned int len;

        if (!EVP_DigestFinal_ex(ctx, digest, &len))
            luaL_error(L, "failed to calculate %s digest", target_hash_name);

        void* newbuffer = lua_newbuffer(L, len);
        memcpy(newbuffer, digest, len);
        OPENSSL_free(digest);
        return 1;
    }

    luaL_error(L, "unhandled HashAlgorithm value %d", algorithm->value);
}
static int computeStringHash(lua_State* L) {
    lua_checkinstance(L, 1, "EncodingService");

    size_t strlen;
    const char* str = luaL_checklstring(L, 2, &strlen);

    auto algorithm = lua_checkenumitem(L, 3, "HashAlgorithm");

    EVP_MD* target_libcrypto_md = NULL;
    const char* target_hash_name = nullptr;

    switch (algorithm->value) {
        case 0: // Blake2b
            if (blake2b == NULL)
                luaL_error(L, "Blake2b fetch failed");

            target_libcrypto_md = blake2b;
            target_hash_name = "blake2b";
            break;
        case 1: { // Blake3
            blake3_hasher_reset(&blake3);

            blake3_hasher_update(&blake3, str, strlen);

            unsigned char output[BLAKE3_OUT_LEN];
            blake3_hasher_finalize(&blake3, output, BLAKE3_OUT_LEN);
            lua_pushlstring(L, reinterpret_cast<const char*>(output), BLAKE3_OUT_LEN);
            return 1;
        }
        case 2: // Md5
            if (md5 == NULL)
                luaL_error(L, "MD5 fetch failed");

            target_libcrypto_md = md5;
            target_hash_name = "md5";
            break;
        case 3: // Sha1
            if (sha1 == NULL)
                luaL_error(L, "SHA1 fetch failed");

            target_libcrypto_md = sha1;
            target_hash_name = "sha1";
            break;
        case 4: { // Sha256
            if (sha256 == NULL)
                luaL_error(L, "SHA256 fetch failed");

            target_libcrypto_md = sha256;
            target_hash_name = "sha256";
            break;
        }
        default:
            break;
    }

    if (target_libcrypto_md != NULL) {
        if (ctx == NULL)
            luaL_error(L, "libcrypto context creation failed");

        if (!EVP_DigestInit_ex(ctx, target_libcrypto_md, NULL))
            luaL_error(L, "failed to initialize %s digest", target_hash_name);

        if (!EVP_DigestUpdate(ctx, str, strlen))
            luaL_error(L, "failed to update %s digest", target_hash_name);

        unsigned char* digest = static_cast<unsigned char*>(OPENSSL_malloc(EVP_MD_get_size(target_libcrypto_md)));
        if (digest == NULL)
            luaL_error(L, "failed to allocate digest");

        unsigned int len;

        if (!EVP_DigestFinal_ex(ctx, digest, &len))
            luaL_error(L, "failed to calculate %s digest", target_hash_name);

        lua_pushlstring(L, reinterpret_cast<const char*>(digest), BLAKE3_OUT_LEN);
        OPENSSL_free(digest);
        return 1;
    }

    luaL_error(L, "unhandled HashAlgorithm value %d", algorithm->value);
}

static int getDecompressedBufferSize(lua_State* L) {
    lua_checkinstance(L, 1, "EncodingService");

    size_t bufferlen;
    void* buffer = luaL_checkbuffer(L, 2, &bufferlen);

    auto algorithm = lua_checkenumitem(L, 3, "CompressionAlgorithm");

    switch (algorithm->value) {
        case 0: { // Zstd
            size_t capacity = ZSTD_getFrameContentSize(buffer, bufferlen);
            if (capacity > MAXSSIZE)
                lua_pushnil(L);
            else
                lua_pushinteger(L, capacity);
            return 1;
        }
    }
    luaL_error(L, "unhandled HashAlgorithm value %d", algorithm->value);
}

};

void rbxInstance_EncodingService_init() {
    auto this_class = std::make_shared<rbxClass>();
    this_class->name.assign("EncodingService");
    this_class->tags |= rbxClass::NotCreatable;
    this_class->superclass = rbxClass::class_map.at("Instance");

    this_class->newMethod("Base64Decode", rbxInstance_EncodingService_methods::base64Decode);
    this_class->newMethod("Base64Encode", rbxInstance_EncodingService_methods::base64Encode);
    this_class->newMethod("CompressBuffer", rbxInstance_EncodingService_methods::compressBuffer);
    this_class->newMethod("DecompressBuffer", rbxInstance_EncodingService_methods::decompressBuffer);
    this_class->newMethod("ComputeBufferHash", rbxInstance_EncodingService_methods::computeBufferHash);
    this_class->newMethod("ComputeStringHash", rbxInstance_EncodingService_methods::computeStringHash);
    this_class->newMethod("GetDecompressedBufferSize", rbxInstance_EncodingService_methods::getDecompressedBufferSize);

    rbxClass::class_map.try_emplace("EncodingService", this_class);
    ServiceProvider::registerService("EncodingService");

    {
    Enum enums;
    enums.name = "CompressionAlgorithm";

    {
    EnumItem item;
    item.name = "Zstd";
    item.enum_name = enums.name;
    item.value = 0;

    enums.item_map.try_emplace(item.name, item);
    }

    Enum::enum_map.try_emplace(enums.name, enums);
    }

    {
    Enum enums;
    enums.name = "HashAlgorithm";

    {
    EnumItem item;
    item.name = "Blake2b";
    item.enum_name = enums.name;
    item.value = 0;

    enums.item_map.try_emplace(item.name, item);
    }
    {
    EnumItem item;
    item.name = "Blake3";
    item.enum_name = enums.name;
    item.value = 1;

    enums.item_map.try_emplace(item.name, item);
    }
    {
    EnumItem item;
    item.name = "Md5";
    item.enum_name = enums.name;
    item.value = 2;

    enums.item_map.try_emplace(item.name, item);
    }
    {
    EnumItem item;
    item.name = "Sha1";
    item.enum_name = enums.name;
    item.value = 3;

    enums.item_map.try_emplace(item.name, item);
    }
    {
    EnumItem item;
    item.name = "Sha256";
    item.enum_name = enums.name;
    item.value = 4;

    enums.item_map.try_emplace(item.name, item);
    }

    Enum::enum_map.try_emplace(enums.name, enums);
    }

    ctx = EVP_MD_CTX_new();
    blake3_hasher_init(&blake3);
    blake2b = EVP_MD_fetch(NULL, "BLAKE2b512", NULL);
    md5 = EVP_MD_fetch(NULL, "MD5", NULL);
    sha1 = EVP_MD_fetch(NULL, "SHA1", NULL);
    sha256 = EVP_MD_fetch(NULL, "SHA256", NULL);
}

void rbxInstance_EncodingService_cleanup() {
    EVP_MD_free(blake2b);
    EVP_MD_free(md5);
    EVP_MD_free(sha1);
    EVP_MD_free(sha256);
    EVP_MD_CTX_free(ctx);
}

}; // namespace frostbyte
