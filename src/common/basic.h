#pragma once
#define DLB_MATH_EXTERN
#include "dlb/dlb_math.h"
#include <cstdint>

#define FDOV_FULLSCREEN 0

// TODO: These need to be queried from rendersystem or something
#if FDOV_FULLSCREEN
#define SCREEN_W 1920
#define SCREEN_H 1080
#else
#define SCREEN_W 1600
#define SCREEN_H 900
#endif

typedef uint32_t UID;

#define C255(color) {\
    color.r * 255, \
    color.g * 255, \
    color.b * 255, \
    color.a * 255  \
}

#define C_GRASS { 38, 43, 25, 255 }