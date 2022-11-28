#pragma once
#include "facet.h"
#include "../common/basic.h"
#include "SDL/SDL_audio.h"

struct Sound : public Facet {
    SDL_AudioSpec  spec      {};
    uint8_t       *audio_buf {};
    uint32_t       audio_len {};
};