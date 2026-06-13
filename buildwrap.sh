#!/bin/bash

if [ -f ".test_success" ]; then
    rm .test_success
fi

clear

pushd frostbyte && ./mate ../frostbyte-desktop/dependencies/rlImGui && popd || exit 1
pushd frostbyte-desktop && ./mate ../frostbyte && popd || exit 1
