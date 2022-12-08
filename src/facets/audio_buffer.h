#pragma once
#include "facet.h"

struct AudioBuffer : public Facet {
    const char    *filename    {};
    SDL_AudioSpec  spec        {};
    uint8_t       *data        {};
    size_t         data_length {};  // length of data buffer in bytes
};