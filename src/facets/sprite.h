#pragma once
#include "facet.h"

struct Depot;

struct Sprite : public Facet {
    //vec2 ssize  {};
    //float scale {};
    vec4         color          {};  // 0.0f - 255.0f
    const char * spritesheetKey {};
    const char * animationKey   {};
    int          frame          {};

    void UpdateRect(Depot &depot);

    inline rect GetSrcRect(void) {
        return srcRect;
    }
    SDL_Texture *GetSDLTexture(void) {
        return cached_sdl_texture;
    }

private:
    SDL_Texture *cached_sdl_texture {};  // idk if this is safe, but it saves two GetFacet() calls per card, per frame
    rect         srcRect            {};
};
