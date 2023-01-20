#pragma once

#define DLB_MATH_EXTERN
#include "dlb/dlb_math.h"
#include "dlb/dlb_rand.h"
#include "dlb/dlb_hash.h"

#include "SDL3/SDL.h"
#include "SDL3/SDL_ttf.h"

#include "soloud.h"
#include "soloud_wav.h"

#include "fbg/ResourceDB_generated.h"
#include "fbg/SaveFile_generated.h"

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

#define FDOV_VSYNC                          1
#define FDOV_FULLSCREEN                     0
#define FDOV_SHOW_TODO_LIST                 1
#define FDOV_STACK_CARDS                    0
#define FDOV_DRAG_BBOX                      0
#define FDOV_DEBUG_BBOX                     0
#define FDOV_PHYSICS_CONSTRAIN_TO_SCREEN    1
#define FDOV_DEBUG_BODY                     0
#define FDOV_DEBUG_GLYPH_ATLAS              0

// TODO: These need to be queried from rendersystem or something
#if FDOV_FULLSCREEN
#define SCREEN_W 1920
#define SCREEN_H 1080
#else
#define SCREEN_W 1728
#define SCREEN_H 972
#endif

typedef uint32_t UID;
struct Depot;

//vec4 cBlack  = {   0,   0,   0, 255 };
//vec4 cWhite  = { 255, 255, 255, 255 };
//vec4 cBeige  = { 224, 186, 139, 255 };
//vec4 cPink   = { 255, 178, 223, 255 };
//vec4 cPurple = {  55,  31,  69, 255 };
//vec4 cGreen  = { 147, 255, 155, 255 };
//vec4 cBlue   = { 130, 232, 255, 255 };
//vec4 cYellow = { 255, 232, 150, 255 };
//vec4 cOrange = { 255, 124,  30, 255 };

#define C255(color) {\
    color.r * 255, \
    color.g * 255, \
    color.b * 255, \
    color.a * 255  \
}

//#define C_GRASS { 49, 134, 112, 255 }  // blue-green
#define C_GRASS { 18, 100, 38, 255 }     // green
//#define C_GRASS { 255, 249, 216, 255 } // wheat

enum Error {
    E_SUCCESS,         // no error
    E_INIT_FAILED,     // init
    E_EOF,             // reached EOF prematurely
    E_UNEXPECTED_CHAR, // unexpected character encountered
    E_IO_ERROR,        // failed to read/write file
    E_VERIFY_FAILED,   // for flatbuffers
};
