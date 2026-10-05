#!/usr/bin/env bash

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DIR=$( realpath "$DIR/.." )

emcmake cmake -B buildwasm -S . -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_FIND_ROOT_PATH=$DIR/install \
  -DOPENSSL_INCLUDE_DIR=$DIR/install/include \
  -DOPENSSL_CRYPTO_LIBRARY=$DIR/install/lib/libcrypto.a \
  -DOPENSSL_SSL_LIBRARY=$DIR/install/lib/libssl.a \
  -DOPENSSL_USE_STATIC_LIBS=TRUE
