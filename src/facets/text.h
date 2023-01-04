#pragma once
#include "facet.h"

#define C_BLACK   "`0"
#define C_BLUE    "`1"
#define C_GREEN   "`2"
#define C_CYAN    "`3"
#define C_RED     "`4"
#define C_MAGENTA "`5"
#define C_YELLOW  "`6"
#define C_WHITE   "`7"

enum TextAlign {
    TextAlign_VTop_HLeft,
    TextAlign_VBottom_HCenter,
};

struct TextCache {
    // values of relevant properties when cached texture was last generated
    UID   font {};
    char *str  {};

    void Destroy(void) {
        free(str);
        *this = {};
    }
};

struct Text : public Facet {
    UID         font   {};  // will invalidate cache
    vec4        color  {};  // will invalidate cache
    const char *str    {};  // will invalidate cache
    TextCache   cache  {};  // created/used by RenderSystem

    TextAlign   align  {};  // how to calculate relative offset of text
    vec2        offset {};  // apply arbitrary offset from calculated position

    inline bool isDirty(void)
    {
        return
            str != cache.str ||
            font != cache.font ||
            strcmp(str, cache.str);
    }
};