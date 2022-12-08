#pragma once
#include "facet.h"
#include "../common/basic.h"

struct CachedSprite {
    // values of relevant properties when cached texture was last generated
    SDL_Surface *surface {};

    // cached texture info
    SDL_Texture *texture     {};
    vec2         textureSize {};

    void Destroy(void) {
        SDL_DestroyTexture(texture);
        *this = {};
    }
};

struct Sprite : public Facet {
    SDL_Surface   *surface {};
    CachedSprite   cache   {};

    vec2 size        {};
    vec4 color       {};
    vec4 attackColor {};
    vec4 defendColor {};
    //float scale {};

    inline bool isDirty(void)
    {
        return surface != cache.surface;
    }
};