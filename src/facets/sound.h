#pragma once
#include "facet.h"

struct Sound : public Facet {
    const char  *soundKey {};
    SoLoud::Wav *wav      {};
    //SDL_AudioSpec  spec        {};
    //uint8_t       *data        {};
    //uint32_t       data_length {};  // length of data buffer in bytes
};