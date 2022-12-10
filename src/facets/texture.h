#pragma once
#include "facet.h"

struct Texture : public Facet {
    const char  *filename    {};
    vec2         size        {};
    SDL_Texture *sdl_texture {};
};