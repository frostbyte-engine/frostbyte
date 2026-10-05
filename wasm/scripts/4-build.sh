#!/usr/bin/env

cmake --build buildwasm -j$(( $(nproc) - 2 ))
