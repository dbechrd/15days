#pragma once
#define DLB_MATH_EXTERN
#include "dlb/dlb_math.h"
#include <cstdint>

// TODO: These need to be queried from rendersystem or something
#define SCREEN_W 1600
#define SCREEN_H 900

typedef uint32_t UID;

#define C255(color) {\
    color.r * 255, \
    color.g * 255, \
    color.b * 255, \
    color.a * 255  \
}