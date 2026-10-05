#!/usr/bin/env bash

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )

git clone --depth 1 --branch v1.8.1 https://github.com/libgit2/libgit2.git || exit 1
cd libgit2 || exit 1

emcmake cmake -B build-wasm -S . \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_FLAGS="-Wno-error=incompatible-pointer-types" \
  -DCMAKE_INSTALL_PREFIX=$DIR/install \
  -DBUILD_SHARED_LIBS=OFF \
  -DBUILD_TESTS=OFF -DBUILD_CLI=OFF -DBUILD_EXAMPLES=OFF \
  -DUSE_THREADS=OFF \
  -DUSE_HTTPS=OFF -DUSE_SSH=OFF -DUSE_NTLMCLIENT=OFF -DUSE_ICONV=OFF \
  -DUSE_BUNDLED_ZLIB=ON \
  -DUSE_HTTP_PARSER=builtin \
  -DREGEX_BACKEND=builtin || exit 1

cmake --build build-wasm -j$(nproc) || exit 1
cmake --install build-wasm || exit 1
