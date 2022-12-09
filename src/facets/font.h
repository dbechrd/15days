#pragma once
#include "facet.h"

struct Font : public Facet {
    std::string  filename {};
    int          ptsize   {};
    TTF_Font    *ttf_font {};
};