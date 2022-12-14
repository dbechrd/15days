#pragma once

#define DLB_MATH_EXTERN
#include "dlb/dlb_math.h"
#include "dlb/dlb_rand.h"
#include "dlb/dlb_hash.h"

#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

#include "soloud.h"
#include "soloud_wav.h"

#include <cassert>
#include <cstdint>
#include <cstdio>

#include <array>
#include <bitset>
#include <deque>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#define FDOV_VSYNC             1
#define FDOV_FULLSCREEN        0
#define FDOV_DRAG_BBOX         0
#define FDOV_DEBUG_BBOX        0
#define FDOV_DEBUG_BODY        0
#define FDOV_DEBUG_GLYPH_ATLAS 0

// TODO: These need to be queried from rendersystem or something
#if FDOV_FULLSCREEN
#define SCREEN_W 1920
#define SCREEN_H 1080
#else
#define SCREEN_W 1600
#define SCREEN_H 900
#endif

typedef uint32_t UID;
struct Depot;

#define C255(color) {\
    color.r * 255, \
    color.g * 255, \
    color.b * 255, \
    color.a * 255  \
}

//#define C_GRASS { 49, 134, 112, 255 }  // blue-green
#define C_GRASS { 18, 100, 38, 255 }     // green
//#define C_GRASS { 255, 249, 216, 255 } // wheat
