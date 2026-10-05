#!/usr/bin/env bash

BUILD_DIR=$(pwd)/buildwasm

DIR=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
DIR="$DIR/../server"

cd $DIR || exit 1

if [ -f "frostbyte-desktop.data" ]; then
  rm frostbyte-desktop.data
fi
if [ -f "frostbyte-desktop.wasm" ]; then
  rm frostbyte-desktop.wasm
fi
if [ -f "frostbyte-desktop.js" ]; then
  rm frostbyte-desktop.js
fi

ln -s $BUILD_DIR/frostbyte-desktop/frostbyte-desktop.data ./frostbyte-desktop.data || exit 1
ln -s $BUILD_DIR/frostbyte-desktop/frostbyte-desktop.wasm ./frostbyte-desktop.wasm || exit 1
ln -s $BUILD_DIR/frostbyte-desktop/frostbyte-desktop.js ./frostbyte-desktop.js || exit 1

python -m http.server 8080

