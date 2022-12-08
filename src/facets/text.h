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
    UID   font  {};
    vec4  color {};
    char *str   {};

    // cached texture info
    SDL_Texture *texture     {};
    vec2         textureSize {};

    void Destroy(void) {
        free((void *)str);
        SDL_DestroyTexture(texture);
        *this = {};
    }
};

struct Text : public Facet {
    UID         font   {};  // will invalidate cache
    vec4        color  {};  // will invalidate cache
    const char *str    {};  // will invalidate cache
    CachedText  cache  {};  // created/used by RenderSystem

    TextAlign   align  {};  // how to calculate relative offset of text
    vec2        offset {};  // apply arbitrary offset from calculated position

    inline bool isDirty(void)
    {
        return
            font != cache.font ||
            !v4_equals(&color, &cache.color) ||
            strcmp(str, cache.str);
    }
};