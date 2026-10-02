#pragma once

#ifdef FROSTBYTE_HEADLESS

typedef struct Vector2 {
    float x;
    float y;
} Vector2;
typedef struct Vector3 {
    float x;
    float y;
    float z;
} Vector3;

typedef struct Vector4 {
    float x;
    float y;
    float z;
    float w;
} Vector4;

typedef struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
} Color;

typedef struct Rectangle {
    float x;
    float y;
    float width;
    float height;
} Rectangle;
#else

#include "raylib.h"

#endif
