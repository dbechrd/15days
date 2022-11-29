#pragma once
#include "facet.h"
#include "../common/basic.h"
#include "SDL/SDL.h"
#include "SDL/SDL_ttf.h"

enum TextAlign {
    TextAlign_VTop_HLeft,
    TextAlign_VBottom_HCenter,
};

struct CachedText {
    // values of relevant properties when cached texture was last generated
    TTF_Font  *font  {};
    char      *text  {};
    vec4       color {};

    // cached texture info
    SDL_Texture *tex      {};
    vec2         texSize  {};
};

struct Text : public Facet {
    TTF_Font   *font   {};  // will invalidate cache
    const char *text   {};  // will invalidate cache
    TextAlign   align  {};  // how to calculate relative offset of text
    vec4        color  {};  // will invalidate cache
    CachedText  cache  {};  // created/used by RenderSystem

    bool isDirty(void);
};