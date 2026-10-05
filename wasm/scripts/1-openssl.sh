#!/usr/bin/env bash

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DIR=$( realpath "$DIR/.." )

cd $DIR || exit 1

git clone --depth 1 --branch openssl-3.0.13 https://github.com/openssl/openssl.git || exit 1
cd openssl || exit 1

emconfigure ./Configure linux-generic32 no-asm no-threads no-shared no-sock no-dso no-engine no-ui-console no-tests \
  --prefix=$DIR/install --openssldir=$DIR/install/ssl \
  -DOPENSSL_SYS_NETWARE -D__STDC_NO_ATOMICS__ || exit 1

sed -i 's|^CROSS_COMPILE.*|CROSS_COMPILE=|' Makefile || exit 1
# emmake make -j$(nproc) build_generic_sources || exit 1
emmake make install_dev || exit 1
