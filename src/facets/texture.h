#pragma once
#include "facet.h"

struct Texture : public Facet {
    std::string  filename    {};
    vec2         size        {};
    SDL_Texture *sdl_texture {};
};