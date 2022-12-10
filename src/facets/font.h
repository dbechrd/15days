#pragma once
#include "facet.h"

struct Font : public Facet {
    const char *filename {};
    int         ptsize   {};
    TTF_Font   *ttf_font {};
};